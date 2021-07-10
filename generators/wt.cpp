/* Copyright 2021 Yegor Suslin
 *
 * This file is part of Kraps library.
 *
 * Kraps is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kraps is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kraps.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <memory>
#include <cmath>
#include <cstring>

#include "wt.hpp"
#include "../serialize/base64/base64.hpp"


namespace kraps {

Wavetable::Wavetable(uint16_t waveform_size) : Generator (p_wt, 1, 0)
{
    this->waveform_size = waveform_size;
    phase_cst = float8 ( (float) this->waveform_size / ( 2.0 * M_PI ) );

    pt.add_parameter(new parameter::Parameter<float>("wt_pos", 0, 0, 0, 100));

    io_description[0].push_back({ kWtShiftIn, "SHIFT", "Shifts WT position forward" });

}

Wavetable::~Wavetable()
{
    
}

inline float8 Wavetable::pack_voices(const int8& oct, const float8& pos, const float8& _shift)
{
    int oct_data[8];
    oct.storeu(oct_data);

    int8 pos_int = static_cast <__m256> ( float8ops::roundneg (pos) );
    float8 pos_frac = pos - static_cast <__m256i> (pos_int);

    int8 shift_round = static_cast <__m256> (_shift);
    int8 shift_upper = int8ops::smin (shift_round + int8 (1), n_forms - 1);

    float8 shift_frac = _shift - static_cast <__m256i> (shift_round);

    int8 pos_int_inc = pos_int + int8 (1);
    pos_int_inc = int8ops::andnot (waveform_size, pos_int_inc); 

    int pos_int_data[16];
    int shift_up[8];

    pos_int.storeu(pos_int_data);
    pos_int_inc.storeu( &pos_int_data[8] );
    shift_round.storeu(shift_up);

    float data[16];

    for (int i = 0; i < 16; i++)
        data[i] = forms[shift_up[i & 0b0111]][oct_data[i & 0b0111]][pos_int_data[i]]; // HOT THING
    
    float8 d1 = d1.loadu(data), d2 = d2.loadu( &data[8] );
    float8 s1 = d1 + (d2 - d1) * pos_frac; // HOT THING

    shift_upper.storeu(shift_up);

    for (int i = 0; i < 16; i++)
        data[i] = forms[shift_up[i & 0b0111] ][oct_data[i & 0b0111]][pos_int_data[i]]; // HOT THING TOO
    
    float8 d3 = d3.loadu(data), d4 = d4.loadu( &data[8] );
    float8 s2 = d3 + (d4 - d3) * pos_frac; // HOT THING

    return s1 + (s2 - s1) * shift_frac;

}

void Wavetable::process_callback() 
{

    set_freq();

    float8 phase_cvt    = phase_cst * phase;
    float8 shift_transform = shift + *inputs[kWtShiftIn];

    shift_transform *= (table_size - waveform_size) / waveform_size;
    int8 num_oct = static_cast <__m256> ( float8ops::clamp(float8ops::slog2(freq * float8(2 * waveform_size / sample_rate)), 0, n_tables - 1) );

    float8 o1 = pack_voices(num_oct, phase_cvt, shift_transform);

    *outputs[kGenAudioOut] = o1;
    *outputs[kGenPhaseOut] = phase;

    inc_phase ();
}


void Wavetable::fill_table_from_buffer (float* buf, uint32_t len)
{
    set_lock();

    table_size = (int) len / waveform_size;

    if (table_size < 1)
        return;

    table_size *= waveform_size;

    table.reset(new double[table_size]);

    for (uint32_t i = 0; i < table_size; i++)
        table[i] = buf[i];


    fill_mipmap();

    set_unlock();
}

void Wavetable::fill_table_from_buffer(double* buf, uint32_t len)
{
    set_lock();

    table_size = (int)len / waveform_size;

    if (table_size < 1)
        return;

    table_size *= waveform_size;

    table.reset(new double[table_size]);

    for (uint32_t i = 0; i < table_size; i++)
        table[i] = buf[i];

    fill_mipmap();

    set_unlock();
}

void Wavetable::fill_table_from_fcn (double (*fcn) (double phase))
{
    set_lock();

    table.reset(new double[waveform_size]);

    table_size = waveform_size;

    for (uint32_t i = 0; i < table_size; i++)
		table[i] = (*fcn) ( 2 * M_PI * ( (float) i  / waveform_size ) );
     
    fill_mipmap();

    set_unlock();
}


double* Wavetable::get_table_view() const
{
    return table.get();
}
uint16_t Wavetable::get_wform_size()
{
    return waveform_size;
}

void Wavetable::fill_mipmap () 
{

    uint16_t wf_sz = waveform_size;
    uint32_t nfft = wf_sz / 2;
    kissfft<double> fft_driver (nfft, false);
    std::unique_ptr<std::complex<double>[]> fft_buf = std::make_unique <std::complex <double>[]>(wf_sz);
    std::unique_ptr<std::complex<double>[]> fft_buf_inv = std::make_unique <std::complex <double>[]>(wf_sz);    

    n_forms = table_size / waveform_size;
    n_tables = log2(nfft);

    forms.clear();
    forms.resize(n_forms);

    double mult = 2.0f / wf_sz;
    for (int i = 0; i < n_forms; ++i)
    {     
        for (int j = 0; j < n_tables; ++j)
        {
            forms[i].emplace_back(std::make_unique<double[]>(wf_sz));
            std::memcpy(forms[i][j].get(), table.get() + i * waveform_size, wf_sz * sizeof(double));

            fft_driver.transform_real(forms[i][j].get(), fft_buf.get());

            uint16_t bins = nfft / pow(2, j); // 1024 harmonics; 512; 256; 128; 64; 32; 16; 8; 4; 2; 1

            for (uint32_t s = bins; s < wf_sz; ++s)
                fft_buf[s] = 0.0;

            fft_driver.assign(wf_sz, true);
            fft_driver.transform(fft_buf.get(), fft_buf_inv.get());

            for (uint32_t s = 0; s < wf_sz; ++s)
                forms[i][j][s] = fft_buf_inv[s].real() * mult;

            fft_driver.assign(nfft, false);
        }
        
    }

}

void Wavetable::process_params()
{
    Generator::process_params();

    if (table_size == waveform_size)
        shift = float8(0);
    if (waveform_size < table_size)
        shift = pt.get_raw_value("wt_pos") / float8(100.0);

}

float8 Wavetable::get_shift()
{
    float8 cur_shift = shift + *inputs[kWtShiftIn];
    return cur_shift;
}

uint32_t Wavetable::get_table_size()
{
    return table_size;
}


const nlohmann::json Wavetable::get_serialize_obj()
{
    set_lock();
    nlohmann::json o;
    o["table"] = base64_encode((BYTE*) table.get(), sizeof(double) * table_size);
    o["table_size"] = table_size;
    o["waveform_size"] = waveform_size;

    set_unlock();
    o.update(Processor::get_serialize_obj());

    return o;
}

void Wavetable::set_serialize(const nlohmann::json& obj)
{
    Processor::set_serialize(obj);

    set_lock();

    if (obj.find("table_size") != obj.end())
        obj["table_size"].get_to(table_size);

    if (obj.find("waveform_size") != obj.end()) 
        obj["waveform_size"].get_to(waveform_size);

    set_unlock();

    if (obj.find("table") != obj.end())
    {
        auto decoded = base64_decode(obj["table"]);

        if (decoded.size() / sizeof(double) != table_size)
            fill_table_from_fcn([](double _phase) -> double {
            return sin(_phase); });

        else fill_table_from_buffer((double*) decoded.data(), table_size);
    }

    
}

}
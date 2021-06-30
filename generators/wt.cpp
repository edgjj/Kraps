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
    phase_cst = float8 ( (this->waveform_size - 1) / ( 2.0 * M_PI ) );

    pt.add_parameter(new parameter::Parameter<float>("wt_pos", 0, 0, 0, 100));

    io_description[0].push_back({ kWtShiftIn, "SHIFT", "Shifts WT position forward" });
}

Wavetable::~Wavetable()
{
    
}

inline float8 Wavetable::pack_voices(const float8& oct, const float8& pos, const float8& shift)
{
    // really bad solution, just prototype
    float oct_data[8];
    oct.store(oct_data);

    float8 wform_size = waveform_size;
    float8 t_size = table_size;

    float8 shift_pure = shift / wform_size;
    
    float8 pos_int = roundneg(pos);

    float8 pos_frac = pos - pos_int;

    float8 shift_round = roundneg(shift_pure);
    float8 shift_frac = shift_pure - shift_round;
    pos_int += shift_round * wform_size;
    
    float8 shift_mask = shift_frac > float8(0.f);

    float8 pos_int_inc = pos_int + float8(1);
    pos_int_inc = blend(pos_int_inc, pos_int_inc - wform_size, pos_int_inc >= t_size);

    float pos_int_data[16];

    pos_int.store(pos_int_data);
    pos_int_inc.store( &pos_int_data[8] );

    float data[16];

    for (int i = 0; i < 16; i++)
        data[i] = tables[oct_data[i % 8]][pos_int_data[i]];

    float8 d1 = d1.loadu(data), d2 = d2.loadu( &data[8] );

    float8 one = 1;
    float8 s1 = d1 * (one - pos_frac) + d2 * pos_frac; 

    float8 pos_int_shift = (pos_int + wform_size) % t_size;
    float8 pos_int_shift_inc = (pos_int_inc + wform_size) % t_size;

    pos_int_shift.store(pos_int_data);
    pos_int_shift_inc.store( &pos_int_data[8] );

    for (int i = 0; i < 16; i++)
        data[i] = tables[oct_data[i % 8]][pos_int_data[i]];
    

    float8 d3 = d3.loadu(data), d4 = d4.loadu( &data[8] );
    float8 s2 = d3 * (one - pos_frac) + d4 * pos_frac;


    return blend (s1, s1 * (float8(1) - shift_frac) + s2 * shift_frac, shift_mask);

}

void Wavetable::process_callback()
{

    set_freq();

    float8 num_tables = tables.size() - 1;

    float8 phase_cvt    = phase_cst * phase;
    
    float8 shift_transform = shift + *inputs[kWtShiftIn];

    shift_transform *= table_size - waveform_size;
    shift_transform = clamp(shift_transform, float8(0), float8(table_size - waveform_size));

    float8 num_oct      = clamp (num_tables - slog2 (float8(88200) / freq), 0, num_tables ); 

    float8 no_strip = roundneg (num_oct);
    float8 no_strip_inc = no_strip + float8(1);

    float8 oct_frac = num_oct - no_strip;

    float8 o1 = pack_voices(no_strip, phase_cvt, shift_transform);
    float8 o2 = pack_voices(no_strip_inc, phase_cvt, shift_transform);

    *outputs[kGenAudioOut] = o1 * (float8 (1) - oct_frac) + o2 * oct_frac;
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



void Wavetable::fill_mipmap () // incorrect too
{

    uint32_t wt_sz = table_size;
    uint32_t nfft = wt_sz / 2;
    kissfft<double> fft_driver (nfft, false);
    std::unique_ptr<std::complex<double>[]> fft_buf = std::make_unique <std::complex <double>[]>(wt_sz);
    std::unique_ptr<std::complex<double>[]> fft_buf_inv = std::make_unique <std::complex <double>[]>(wt_sz);

    uint16_t wf_sz = waveform_size;
    int i = 0;
    tables.clear();

    // i think there's a bug somewhere: some tables are being shifted forward, leaving a "pop" in the start;

    while (wf_sz != 0)
    {
        tables.emplace_back( std::make_unique <double[]>(wt_sz) );

        std::memcpy(tables[i].get(), table.get(), wt_sz * sizeof(double));

        fft_driver.transform_real(tables[i].get(), fft_buf.get());

        uint16_t bins = nfft / (2.0 * pow(2, i));  // 1/2 fnyq; 1/4 fnyq; 1/8; 1/16; 1/32; 1/64; 1/128; 1/256; 1/512; 1/1024; ... 1 / waveform_size

        for (uint32_t j = bins; j < wt_sz; ++j)
            fft_buf[j] = 0.0;

        fft_driver.assign(wt_sz, true);
        fft_driver.transform(fft_buf.get(), fft_buf_inv.get());

        double mult = 2.0f / wt_sz;

        for (uint32_t j = 0; j < wt_sz; ++j)
            tables[i][j] = fft_buf_inv[j].real() * mult;

        fft_driver.assign(nfft, false);

        wf_sz /= 2;
        i++;
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
            fill_table_from_fcn([](double phase) -> double {
            return sin(phase); });

        else fill_table_from_buffer((double*) decoded.data(), table_size);
    }

    
}

}
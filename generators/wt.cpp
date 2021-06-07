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
    phase_cst = this->waveform_size / ( 2.0 * M_PI );

    params.push_back(0.0);
    params_constrainments.push_back(std::pair<double, double>(0.0, 100));

    io_description[0].push_back({ kWtShiftIn, "SHIFT", "Shifts WT position forward" });
}

Wavetable::~Wavetable()
{
    
}

float8 Wavetable::pack_voices(const float8& oct, const float8& pos)
{
    float data[8], oct_data[8], pos_data[8];

    oct.store(oct_data);
    pos.store(pos_data);

#pragma loop(hint_parallel(8))
    for (int i = 0; i < 8; i++)
        data[i] = tables[oct_data[i]][pos_data[i]];

    float8 ret = ret.loadu(data);

    return ret;
}

void Wavetable::process_callback()
{
    if (sample_rate == 0.0 || inputs[kGenFreqIn]->src->id == -1)
        return;

    set_freq();

    float8 phase_cvt    = float8 (phase_cst) * phase;
    
    float8 shift_in = *inputs[kWtShiftIn];

    float8 pos_int    = roundneg (phase_cvt + shift_in + float8 (shift));
    float8 pos_int_inc = pos_int + float8(1);

    pos_int_inc = blend(pos_int_inc, pos_int_inc - float8(waveform_size), pos_int_inc >= float8(table_size));

    float8 pos_frac     = phase_cvt - roundneg(phase_cvt);
    float8 log_arg      = freq * float8(table_size / sample_rate); 

    float8 num_oct      = clamp (slog2 (log_arg), 0, NUM_OCTAVES - 1);

    float8 no_strip = roundneg (num_oct);
    float8 no_strip_inc = no_strip + float8(1);
    float8 oct_frac = num_oct - no_strip;

    float8 o1 = pack_voices (no_strip, pos_int_inc) * pos_frac + pack_voices (no_strip, pos_int) * ( float8(1) - pos_frac); // resolve this for SIMD 
    float8 o2 = pack_voices(no_strip_inc, pos_int_inc) * pos_frac + pack_voices(no_strip_inc, pos_int) * (float8(1) - pos_frac); // resolve this for SIMD 

    *outputs[kGenAudioOut] = o1 * (float8 (1) - oct_frac) + o2 * oct_frac;
    *outputs[kGenPhaseOut] = phase;

    inc_phase ();
}

void Wavetable::fill_table_from_buffer (float* buf, uint32_t len)
{
    set_lock();

    table_size = len;
    table.reset(new double[table_size]);

    for (uint32_t i = 0; i < table_size; i++)
        table[i] = buf[i];

    fill_mipmap();

    set_unlock();
}

void Wavetable::fill_table_from_buffer(double* buf, uint32_t len)
{
    set_lock();

    if (len < waveform_size)
        return;

    table_size = len;
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

    // fs = 44100; Nfft; Nfft / 2; Nfft / 4; Nfft / 8 .. others

    for (int i = 0; i < NUM_OCTAVES; i++)
    {

        tables[i] = std::make_unique <double []> (wt_sz);
        std::memcpy (tables[i].get(), table.get(), wt_sz * sizeof(double));

        fft_driver.transform_real(tables[i].get(), fft_buf.get());

        uint16_t bins   = nfft / pow(2, i);  // 1/2 fs; 1/4 fs; 1/8; 1/16; 1/32; 1/64; 1/128; 1/256; 1/512; 1/1024; 1/2048; 1/4096
        if (bins == 1) bins = 0;
        for (uint32_t j = bins; j < wt_sz; j++)
            fft_buf[j] = 0.0;

        fft_driver.assign(wt_sz, true);
        fft_driver.transform(fft_buf.get(), fft_buf_inv.get());

        double mult = 2.0f / wt_sz;

        for (uint32_t j = 0; j < wt_sz; ++j) 
            tables[i][j] = fft_buf_inv[j].real() * mult;

        fft_driver.assign(nfft, false);
    } 

}

void Wavetable::process_params()
{
    
    if (table_size == waveform_size)
        shift = 0;
    if (waveform_size < table_size)
        shift = (params[1] / 100.0) * (table_size - waveform_size);

}

uint32_t Wavetable::get_shift()
{
    return shift;
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
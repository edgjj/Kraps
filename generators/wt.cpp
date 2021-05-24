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


void Wavetable::process_callback()
{
    if (SR_cst == 0.0 || inputs[kGenFreqIn]->src->val == 0.0)
        return;

    set_freq();

    double phase_cvt    = phase_cst * phase;

    // shift = (params[1] / 100.0) * (table_size - waveform_size - 1); - something like this for shift

    unsigned int pos_int    = *inputs[kWtShiftIn] + phase_cvt + shift;
    unsigned int pos_int_inc = pos_int + 1;

    if (pos_int_inc == shift + waveform_size)
        pos_int_inc = shift;

    double pos_frac     = phase_cvt - (int)phase_cvt;
    double log_arg      = freq * table_size / sample_rate; // lets think every wt we get is 44100
    double num_oct      = fmax (fmin (log2(log_arg) - 1, NUM_OCTAVES - 1), 0);

    unsigned int no_strip = (unsigned int)num_oct;
    double  oct_frac = num_oct - no_strip;

    double o1 = tables[no_strip][pos_int_inc] * pos_frac + tables[no_strip][pos_int] * (1 - pos_frac);
    double o2 = tables[no_strip + 1][pos_int_inc] * pos_frac + tables[no_strip + 1][pos_int] * (1 - pos_frac);

    *outputs[kGenAudioOut] = o1 * (1 - oct_frac) + o2 * oct_frac;
    *outputs[kGenPhaseOut] = phase;

    inc_phase ();
}

void Wavetable::fill_table_from_buffer (float* buf, uint32_t len)
{
    WAIT_LOCK;

    bool cur_bypass = is_bypassed();
    if (!cur_bypass)
        set_bypassed(true);

    table_size = len;
    table.reset(new double[table_size]);

    for (uint32_t i = 0; i < table_size; i++)
        table[i] = buf[i];

    fill_mipmap();

    if (!cur_bypass)
        set_bypassed(false);
}

void Wavetable::fill_table_from_buffer(double* buf, uint32_t len)
{
    WAIT_LOCK;

    bool cur_bypass = is_bypassed();
    if (!cur_bypass)
        set_bypassed(true);

    if (len < waveform_size)
        return;

    table_size = len;
    table.reset(new double[table_size]);

    for (uint32_t i = 0; i < table_size; i++)
        table[i] = buf[i];

    fill_mipmap();

    if (!cur_bypass)
        set_bypassed(false);
}

void Wavetable::fill_table_from_fcn (double (*fcn) (double phase))
{
    WAIT_LOCK;

    table.reset(new double[waveform_size]);

    table_size = waveform_size;

    for (uint32_t i = 0; i < table_size; i++)
		table[i] = (*fcn) ( 2 * M_PI * ( (float) i  / waveform_size ) );
     
    
    fill_mipmap();

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
    WAIT_LOCK;

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
    WAIT_LOCK;
    if (table_size == waveform_size)
        shift = 0;
    if (waveform_size < table_size)
        shift = (params[1] / 100.0) * (table_size - waveform_size - 1);
}

uint32_t Wavetable::get_shift()
{
    return shift;
}

uint32_t Wavetable::get_table_size()
{
    return table_size;
}


nlohmann::json Wavetable::get_serialize_obj()
{
    nlohmann::json o;
    o["table"] = base64_encode((BYTE*) table.get(), sizeof(double) * table_size);
    o["table_size"] = table_size;
    o["waveform_size"] = waveform_size;
    o.update(Processor::get_serialize_obj());

    return o;
}

void Wavetable::set_serialize(nlohmann::json obj)
{
    Processor::set_serialize(obj);
    if (obj.find("table_size") != obj.end())
        obj["table_size"].get_to(table_size);

    if (obj.find("waveform_size") != obj.end()) 
        obj["waveform_size"].get_to(waveform_size);

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
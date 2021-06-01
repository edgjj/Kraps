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

#include "filtering.hpp"

namespace kraps
{
namespace filters
{
Filter::Filter() : Processor (p_filter, 3, 3)
{
    
    params = { 10000.0, 0.8, 1.0 };
    params_constrainments.push_back(std::pair <double, double>(20, 20000.0));
    params_constrainments.push_back(std::pair <double, double>(0.10, 16.0));
    params_constrainments.push_back(std::pair <double, double>(1.0, 4.0));


    setup_filtering();

    fake_ptr = new double* [2];
    fake_ptr[0] = new double[1];
    fake_ptr[1] = new double[1];


    io_description[0] =
    {
        {kFilterAudioIn, "AUDIO", "Just audio input."},
        {kFilterFreqIn, "FREQ", "Input for frequency CV."},
        {kFilterResIn, "RES", "Input for resonance CV."},
    };

    io_description[1] =
    {
        {kFilterAudioOutLPF, "LPF", "Low-pass filter output."},
        {kFilterAudioOutHPF, "HPF", "High-pass filter output."},
        {kFilterAudioOutBPF, "BPF", "Band-pass filter output."},
    };

}

Filter::~Filter()
{
    delete fake_ptr[1];
    delete fake_ptr[0];
    delete fake_ptr;
}

void Filter::setup_filtering()
{
   
    filters_bank.emplace_back(std::make_unique <Dsp::SmoothedFilterDesign<Dsp::RBJ::Design::LowPass, 2>>(512));
    filters_bank.emplace_back(std::make_unique <Dsp::SmoothedFilterDesign<Dsp::RBJ::Design::HighPass, 2>>(512));
    filters_bank.emplace_back(std::make_unique <Dsp::SmoothedFilterDesign<Dsp::RBJ::Design::BandPass1, 2>>(512));

    for (auto& i : filters_bank)
    {
        f_params = { 44100.0, params[0], params[1] };
        i->setParams(f_params);
    }
        
}

void Filter::recalculate_sr()
{
    
    f_params[0] = sample_rate;
    for (auto& i : filters_bank)
    {
        i->reset();
        i->setParam(Dsp::ParamID::idSampleRate, sample_rate);
    }
       
}

void Filter::process_callback()
{
    if (sample_rate == 0.0)
        return;

    freq = *inputs[kFilterFreqIn] * ((sample_rate - 2000) / 2) + params[0];
    freq = fmax (fmin(freq, (sample_rate - 2000) / 2), 20.0);

    double q = *inputs[kFilterResIn] + params[1];

    float8 data = *inputs[kFilterAudioIn];
    float f8cvt[2];
    float in = data.hadd();
    

    f_params[1] = freq;
    f_params[2] = q;
    
    for (int i = 0; i < filters_bank.size(); i++)
    {
        fake_ptr[0][0] = in;
        fake_ptr[1][0] = in;
        filters_bank[i]->setParams(f_params);
        filters_bank[i]->process(1, fake_ptr);

        f8cvt[0] = fake_ptr[0][0];
        f8cvt[1] = fake_ptr[1][0];

        *outputs[i] = data.loadu(f8cvt, 2);
    }

    // do parallel processing of 4 filter types: LP, HP, BP, AP
    // also we need parameter smoothing
}

}
}
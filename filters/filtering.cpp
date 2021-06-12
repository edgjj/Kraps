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
    pt = kraps::parameter::pt::ParameterTable(
        { new parameter::Parameter<float>("frequency", 10000, 10000, 20, 20000),
         new parameter::Parameter<float>("qfactor", 0.8, 0.8, 0.1, 16.0),
         new parameter::Parameter<int>("order", 1, 1, 1, 4.0),
        });


    setup_filtering();

    fake_ptr = new double* [8];
    for (int i = 0; i < 8; i++)
        fake_ptr[i] = new double[1];

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
    for (int i = 0; i < 8; i++)
        delete [] fake_ptr[i];

    delete [] fake_ptr;
}

void Filter::setup_filtering()
{

    filters_bank.emplace_back(std::make_unique <Dsp::SmoothedFilterDesign<Dsp::RBJ::Design::LowPass, 8>>(512));
    filters_bank.emplace_back(std::make_unique <Dsp::SmoothedFilterDesign<Dsp::RBJ::Design::HighPass, 8>>(512));
    filters_bank.emplace_back(std::make_unique <Dsp::SmoothedFilterDesign<Dsp::RBJ::Design::BandPass1, 8>>(512));

    for (auto& i : filters_bank)
    {
        f_params = { 44100.0, pt.get_raw_value("frequency"),  pt.get_raw_value("qfactor") };
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

void Filter::process_params()
{
    param_freq = pt.get_raw_value("frequency");
    param_qfac = pt.get_raw_value("qfactor");
    param_order = pt.get_raw_value("order");
}

void Filter::process_callback()
{
    if (sample_rate == 0.0)
        return;

    freq = *inputs[kFilterFreqIn] * float8((sample_rate - 2000) / 2) + param_freq;
    freq = clamp(freq, 20.0, (sample_rate - 2000) / 2); // we got avx float and still hadd


    float8 q = *inputs[kFilterResIn] + param_qfac;
    float8 in = *inputs[kFilterAudioIn];
    float u_data[8];

    in.storeu(u_data);

    f_params[1] = freq;
    f_params[2] = q;
    

    // need AVX filters implementation so we dont need to do that this below

    for (int i = 0; i < filters_bank.size(); i++)
    {
        for (int j = 0; j < 8; j++)
            fake_ptr[j][0] = u_data[j];


        filters_bank[i]->setParams(f_params);
        filters_bank[i]->process(1, fake_ptr);

        for (int j = 0; j < 8; j++)
            u_data[j] = fake_ptr[j][0];

        *outputs[i] = in.loadu(u_data);
    }

}

}
}
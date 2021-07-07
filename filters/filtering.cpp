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
Filter::Filter() : Processor (p_filter, 3, 6)
{
    pt = kraps::parameter::pt::ParameterTable(
        { new parameter::Parameter<float>("frequency", 10000, 10000, 20, 20000),
         new parameter::Parameter<float>("qfactor", 0.8, 0.8, 0.1, 16.0),
         new parameter::Parameter<int>("order", 1, 1, 1, 4.0),
        });


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
        {kFilterAudioOutAPF, "APF", "All-pass filter output."},
        {kFilterAudioOutNF, "NOTCH", "Notch filter output."},
        {kFilterAudioOutPF, "PEAK", "Peak filter output."},
    };

    gCoeff = 1.0f;
    RCoeff = 1.0f;
    KCoeff = 0.0f;

    freq = 1000;
    qfac = 0.5f;
    calc_filter();
}

Filter::~Filter()
{
}


void Filter::recalculate_sr()
{
    calc_filter();
}

inline float8 Filter::ftan(const float8& v)
{
    return v * ( float8 (12.56637061435916) + float8(4) * v) / (float8(14.137166941154055) - float8(5.654866776461622) * v * v); // accident tan approximation
}

void Filter::calc_filter()
{

    float8 wd = freq * float8 (2.0f * M_PI);
    float8 T = 1.0f / sample_rate;

    float8 wa = ( 2.0f / T ) * ftan(wd * T / float8 (2.0f) );

    // Calculate g (gain element of integrator)
    gCoeff = wa * T / float8 (2.0f);			// Calculate g (gain element of integrator)

    // Calculate Zavalishin's R from Q (referred to as damping parameter)
    RCoeff = float8 (1.0f) / ( float8 (2.0f) * qfac);

}

void Filter::process_params()
{
    freq = pt.get_raw_value("frequency");
    qfac = pt.get_raw_value("qfactor") + *inputs[kFilterResIn];
    param_order = pt.get_raw_value("order");

    freq += *inputs[kFilterFreqIn] * float8((sample_rate - 2000) / 2) ;
    freq = float8ops::clamp(freq, 2, 0.96 * (sample_rate / 2) );

    calc_filter();
}

void Filter::process_callback()
{
    if (sample_rate == 0.0)
        return;

    float8 in = *inputs[kFilterAudioIn];

    const float8 HP = (in - ( float8 (2.0f) * RCoeff + gCoeff) * z1_A - z2_A)
        / ( float8 (1.0f) + (float8(2.0f) * RCoeff * gCoeff) + gCoeff * gCoeff);

    const float8 BP = HP * gCoeff + z1_A;

    const float8 LP = BP * gCoeff + z2_A;

    const float8 UBP = float8 (2.0f) * RCoeff * BP;
    const float8 Notch = in - UBP;
    const float8 AP = in - ( float8 (4.0f) * RCoeff * BP);
    const float8 Peak = LP - HP;

    z1_A = gCoeff * HP + BP;		// unit delay (state variable)
    z2_A = gCoeff * BP + LP;		// unit delay (state variable)

    *outputs[kFilterAudioOutLPF] = LP;
    *outputs[kFilterAudioOutHPF] = HP;
    *outputs[kFilterAudioOutBPF] = UBP;
    *outputs[kFilterAudioOutAPF] = AP;
    *outputs[kFilterAudioOutNF] = Notch;
    *outputs[kFilterAudioOutPF] = Peak;

}

}
}
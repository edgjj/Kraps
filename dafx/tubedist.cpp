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

#include "tubedist.hpp"

namespace kraps
{
namespace dafx
{
TubeDist::TubeDist() : Processor(p_tube, 1, 1)
{
    pt = kraps::parameter::pt::ParameterTable(
        { new parameter::Parameter<float>("gain", 0, 0, 0.0, 30.0),
        new parameter::Parameter<float>("pre_gain", 0, 0, 0.0, 20.0),
        new parameter::Parameter<float>("top_clip", 0, 0, 0.0, 100.0),
        new parameter::Parameter<float>("bot_clip", 0, 0, 0.0, 100.0),
        new parameter::Parameter<float>("peak_clip", 0, 0, 0.0, 100.0),
        new parameter::Parameter<float>("out_gain", -4.5, -4.5, -60, 10.0),
        });

    process_params();

    io_description[0] =
    {
        {kDAFXAudioIn, "AUDIO", "Just audio input."}
    };

    io_description[1] =
    {
        {kDAFXAudioOut, "AUDIO", "Output for processed signal."}
    };
}
TubeDist::~TubeDist()
{
	;
}
void TubeDist::process_params()
{
    float top_clip = pt.get_raw_value("top_clip");
    tC = (top_clip / 100.0 < 0.01 ? 0.01 : top_clip / 100.0) * 2 + 1;
    bC = (pt.get_raw_value("bot_clip").operator float() / 100.0 + 0.166666667) * 6;
    pC = pt.get_raw_value("peak_clip").operator float() / 100.0 * 8;

    auto d2g = [](double dB) { return pow(10, dB / 20); };
    gain = d2g(pt.get_raw_value("gain"));
    pre_gain = d2g(pt.get_raw_value("pre_gain"));
    out_gain = d2g(pt.get_raw_value("out_gain"));
}

float8 TubeDist::ftanh(const float8& x)
{
    float8 magic = 135135.0f;
    float8 x2 = x * x;
    float8 a = x * (magic + x2 * (float8(17325.0f) + x2 * (float8(378.0f) + x2)));
    float8 b = magic + x2 * (float8 (62370.0f) + x2 * (float8(3150.0f) + x2 * float8(28.0f)));

    return a / b;
}

void TubeDist::process_callback()
{
    float8 in = *inputs[kDAFXAudioIn];
    in *= float8(pre_gain);

    float data[8];
    in.storeu(data);
    
#pragma loop(hint_parallel(8))
    for (int i = 0; i < 8; i++)
    {
        if (data[i] < 0) data[i] = tanh((data[i] / bC) * gain) * bC;

        //scaled biased x^1.5
        data[i] += bC;
        data[i] = data[i] * sqrtf(fabs(data[i])) - bC * sqrtf(fabs(bC)); // x^1.5 = in*(sqrt(abs(in))

        data[i] /= sqrtf(bC) * 1.5;

        if (data[i] >= 0) {
            data[i] *= gain;
            data[i] /= tC;
            data[i] = tanh(data[i]) * (pC + 1) - (data[i] / (sqrtf(data[i] * data[i] + 1))) * pC;
            data[i] *= tC;
        }
    }
    

    *outputs[kDAFXAudioOut] = in.loadu(data) * float8 (out_gain);
}
}
}
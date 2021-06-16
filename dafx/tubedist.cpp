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
    float8 top_clip = pt.get_raw_value("top_clip");
    tC = blend(top_clip, float8(0.01), (top_clip / float8(100.0)) < float8(0.01)) * float8 (2) + float8 (1);
    bC = ( ( pt.get_raw_value("bot_clip") / float8 (100.0) ) + float8 (0.166666667) ) * float8 (6);
    pC = pt.get_raw_value("peak_clip") * float8 (8.0 / 100.0);

    auto d2g = [](double dB) { return pow(10, dB / 20); };
    gain = d2g(pt.get_raw_value("gain"));
    pre_gain = d2g(pt.get_raw_value("pre_gain"));
    out_gain = d2g(pt.get_raw_value("out_gain"));
}

inline float8 TubeDist::ftanh(const float8& x)
{
    // https://www.kvraudio.com/forum/viewtopic.php?f=33&t=521377
    float8 x_trans = x + float8 (0.18) * (x * x * x);
    return x_trans / ssqrt(x_trans * x_trans + float8(1));
}

void TubeDist::process_callback()
{
    float8 in = *inputs[kDAFXAudioIn];

    in *= pre_gain;
    in = blend(in, ftanh( (in / bC) * gain ) * bC, in < float8(0));
    in += bC;
    in = in * ssqrt(sfabs(in)) - bC * ssqrt(sfabs(bC));
    in /= ssqrt(bC) * float8(1.5);

    float8 top = in;

    top *= gain;
    top /= tC;
    top = ftanh(top) * (pC + float8(1)) - (top / (ssqrt(top * top + float8 (1)))) * pC;
    top *= tC;

    in = blend(in, top, in >= float8(0));

    *outputs[kDAFXAudioOut] = in * float8 (out_gain);
}
}
}
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
    params = { 0.0, 0.0, 0.0, 0.0, 0.0, -4.5 };
    params_constrainments = { {0.0, 30.0}, {0.0, 20.0}, {0.0, 100.0}, {0.0, 100.0}, {0.0, 100.0}, {-60.0, 10.0} };
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
    tC = (params[2] / 100.0 < 0.01 ? 0.01 : params[2] / 100.0) * 2 + 1;
    bC = (params[3] / 100.0 + 0.166666667) * 6;
    pC = params[4] / 100.0 * 8;

    auto d2g = [](double dB) { return pow(10, dB / 20); };
    gain = d2g(params[0]);
    pre_gain = d2g(params[1]);
    out_gain = d2g(params[5]);
}

float8 TubeDist::ftanh(float8 x)
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
    float outsmp = in.hadd() * pre_gain;

    if (outsmp < 0) outsmp = tanh((outsmp / bC) * gain) * bC;

    //scaled biased x^1.5
    outsmp += bC;
    outsmp = outsmp * sqrtf(fabs(outsmp)) - bC * sqrtf(fabs(bC)); // x^1.5 = in*(sqrt(abs(in))

    outsmp /= sqrtf(bC) * 1.5;

    if (outsmp >= 0) {
        outsmp *= gain;
        outsmp /= tC;
        outsmp = tanh(outsmp) * (pC + 1) - (outsmp / (sqrtf(outsmp * outsmp + 1))) * pC;
        outsmp *= tC;
    }


    /*for (int i = 0; i < 2; i++)
    {
        process_dist ()
    }*/

    outsmp *= out_gain;
    *outputs[kDAFXAudioOut] = float8 (outsmp);
}
}
}
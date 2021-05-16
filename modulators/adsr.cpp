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

#include <cmath>

#include "adsr.hpp"

namespace kraps {
ADSR::ADSR () : Processor (p_adsr, 1, 1) // possibly add smoothing for 4 samples behind
{
    params = std::vector<double> (4, 0.001);
    params[3] = 0.0;
    params_constrainments = std::vector<std::pair <double, double >>(3, std::pair<double, double>(0.0, 20.0));
    params_constrainments.push_back(std::pair(-60, 0));

    io_description[0] =
    {
        { kADSRGate, "GATE", "Just gate. Resets ADSR state."}
    };
    io_description[1] =
    {
        { kADSRAudioOut, "OUT", "Gives access to produced control signal."}
    };
}


void ADSR::set_gate(bool g)
{
    if (g == true)
    {
        state = adsr_ENV_ATT;
        pos = 0.0;
    } 
    else 
    {
        state = adsr_ENV_REL;
    }

    gate = g;
}


void ADSR::process_callback()
{
    double ret = pos;

    if (*inputs[kADSRGate] != gate)
    {
        set_gate(*inputs[kADSRGate]);
    }

    
    switch (state){
        case adsr_ENV_ATT:
            if (pos < 1.f && step[adsr_attack] > 0.f){
                pos += step[adsr_attack];
            } else {
                pos = 1.f;
                state = adsr_ENV_DECAY;
            }
            break;
        case adsr_ENV_DECAY:
            if (pos > sustain_amp && step[adsr_decay] > 0.f){
                pos -= step[adsr_decay];
            } else {
                pos = sustain_amp;
                state = adsr_ENV_SUSTAIN;
            }
            break;
        case adsr_ENV_SUSTAIN:
            break;
        case adsr_ENV_REL:
            if (pos > 0.f && step[adsr_release] > 0.f){
                pos -= step[adsr_release];
            }
            else {
                pos = 0.f;
                state = adsr_ENV_IDLE;
            }
            break;
    }

    *outputs[kADSRAudioOut] = ret;
    
}

void ADSR::process_params ()
{
    for (int i = 0; i < params.size(); i++)
    {
        if (i == adsr_sustain)
        {
            sustain_amp = pow(10, params[i] / 20.0);
            continue;
        }

        if (params[i] <= 0.0)
        {
            step[i] = 0.0;
            continue;
        }
        step[i] = 1.0 / (sample_rate * params[i]);
    }
}

}

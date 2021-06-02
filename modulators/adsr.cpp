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


void ADSR::set_gate()
{
    float8 cmp = *inputs[kADSRGate] != gate;   

    if (movemask(cmp) == 0)
        return;
    
    float8 cmp_att = andnot(gate, *inputs[kADSRGate]) == float8 (1.0f);
    float8 cmp_rel = andnot(*inputs[kADSRGate], gate) == float8(1.0f);

    float8 att = adsr_ENV_ATT;
    float8 rel = adsr_ENV_REL;

    state = blend (state, att, cmp_att); // attack
    state = blend (state, rel, cmp_rel); // release

    pos = blend (pos, float8 (0), cmp_att);

    gate = *inputs[kADSRGate];
}

float8 ADSR::get_position()
{
    set_lock();
    float8 ret = pos;
    set_unlock();

    return ret;
    
}

void ADSR::process_callback()
{

    set_gate();

    float8 comp_att = step[adsr_attack] > float8 (0.f) & pos < float8(1.f);
    float8 comp_decay = step[adsr_decay] > float8 (0.f) & pos > float8(sustain_amp);
    float8 comp_rel = step[adsr_release] > float8 (0.f) & pos > float8(0.f);

    float8 att = state == float8(adsr_ENV_ATT);
    float8 dec = state == float8(adsr_ENV_DECAY);
    float8 rel = state == float8(adsr_ENV_REL);

    float8 state_att = blend(float8(0), state, att);
    float8 state_dec = blend(float8(0), state, dec);
    float8 state_rel = blend(float8(0), state, rel);


    state = blend(float8 (adsr_ENV_DECAY) & state_att != float8(0), state_att, comp_att & att )
        + blend(float8(adsr_ENV_SUSTAIN) & state_dec != float8(0), state_dec, comp_decay & dec )
        + blend(float8(adsr_ENV_IDLE) & state_rel != float8(0), state_rel, comp_rel & rel );

    float8 inc(0.0);

    inc = blend(inc, float8(step[adsr_attack]), state == float8(adsr_ENV_ATT));
    inc = blend(inc, float8(-step[adsr_decay]), state == float8(adsr_ENV_DECAY));
    inc = blend(inc, float8(-step[adsr_release]), state == float8(adsr_ENV_REL));

    pos += inc;
    pos = clamp(pos, float8(0), float8(1));

    
    *outputs[kADSRAudioOut] = pos;
    
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
            switch (i)
            {
            case adsr_attack:
                step[i] = 1.0;
                break;
            case adsr_decay:
                step[i] = 1.0 - sustain_amp;
                break;
            default:
                step[i] = sustain_amp;
                break;
            }
            
            continue;
        }
        step[i] = 1.0 / (sample_rate * params[i]);
    }
    
}

}

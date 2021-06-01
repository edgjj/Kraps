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
 
#include <math.h>

#include "generator.hpp"

namespace kraps {

Generator::Generator (uint8_t type, uint8_t i, uint8_t o) : Processor (type, i + 3, o + 2)
{
    params = std::vector<double>(1, 1.0);
    params_constrainments.push_back(std::pair(0.0, 64.0));

    io_description[0] =
    {
        {
            kGenFreqIn, "FREQ", "Raw frequency input."
        },
        {
            kGenPhaseIn, "PHASE", "Phase Modulation input."
        },
        {
            kGenGate, "GATE", "Just gate. Resets generator phase."
        }
    };

    io_description[1] =
    {
        {
            kGenAudioOut, "AUDIO", "Provides raw audio."
        },
        {
            kGenPhaseOut, "PHASE", "Provides raw generator phase."
        }
    };
}

Generator::~Generator ()
{

}

void Generator::set_phase(double phase)
{
    this->phase = phase;
}

void Generator::set_freq () 
{
    float8 raw_freq = *inputs[kGenFreqIn];
    freq = clamp(raw_freq * float8(params[0]), float8(5.38330078125), float8(sample_rate / 2.0));
    phase_inc = freq * freq_cst;
}

void Generator::set_freq(double _freq)
{
    freq = _freq;
    phase_inc = freq * freq_cst;
}

void Generator::set_freq(float8 _freq)
{
    freq = _freq;
    phase_inc = freq * freq_cst;
}

double Generator::get_phase()
{
    return phase;
}
void Generator::inc_phase ()
{
    float8 cmp = *inputs[kGenGate] != gate;

    if (movemask(cmp) != 0)
    {
        float8 cmp_gate = andnot(gate, *inputs[kGenGate]) == float8(1.0f);
        phase = blend(phase, float8(0), cmp_gate);
        gate = *inputs[kGenGate];
    }
    float8 ext_phase = *inputs[kGenPhaseIn];

    phase += phase_inc + ext_phase; //  +

    float8 mpi2 = 2 * M_PI;

    while (movemask(phase < float8(0.f)) != 0)
        phase = blend(phase, phase + mpi2, phase < float8(0.f));

    while (movemask(phase >= mpi2) != 0)
        phase = blend(phase, phase - mpi2, phase >= mpi2);

}

}
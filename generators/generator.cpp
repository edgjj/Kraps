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
    pt = kraps::parameter::pt::ParameterTable(
        { new parameter::Parameter<float>("freq_mult", 1, 1, 0, 64),
        new parameter::Parameter<float>("freq_shift", 0, 0, -200.f, 200.f)
        });


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

void Generator::set_phase(double _phase)
{
    phase = _phase;
}

void Generator::process_params()
{
    freq_mult = pt.get_raw_value("freq_mult");
    freq_shift = pt.get_raw_value("freq_shift");
}

void Generator::set_freq () // for oscillators
{
    float8 raw_voltage = *inputs[kGenFreqIn];
    freq = float8ops::clamp( float8 ( pow256_ps (_mm256_set1_ps (2), raw_voltage) ) * a3_tune * freq_mult + freq_shift, 0, 20000);
    phase_inc = freq * freq_cst;
}

void Generator::set_freq(double _freq) // for everything else
{
    freq = _freq;
    phase_inc = freq * freq_cst;
}

void Generator::set_freq(float8 _freq) // for everything else
{
    freq = _freq;
    phase_inc = freq * freq_cst;
}

float8 Generator::get_phase()
{
    return phase;
}

void Generator::inc_phase ()
{
    using namespace float8ops;
    float8 cmp = *inputs[kGenGate] != gate;

    if (movemask(cmp) != 0)
    {
        float8 cmp_gate = andnot(gate, *inputs[kGenGate]) == float8(1.0f);
        phase_internal = blend(phase_internal, float8(0), cmp_gate);
        gate = *inputs[kGenGate];
    }

    float8 mpi2 = 2 * M_PI;

    float8 ext_phase = *inputs[kGenPhaseIn];
    phase = ext_phase * mpi2 + phase_internal; // we also can accumulate external phase instead of just summing (as it was before);



    phase_internal += phase_inc; 

    while (movemask(phase < float8(0.f)) != 0)
        phase = blend(phase, phase + mpi2, phase < float8(0.f));

    while (movemask(phase >= mpi2) != 0)
        phase = blend(phase, phase - mpi2, phase >= mpi2);

    while (movemask(phase_internal >= mpi2) != 0)
        phase_internal = blend(phase_internal, phase_internal - mpi2, phase_internal >= mpi2);

}

}
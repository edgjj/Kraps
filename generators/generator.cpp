
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
    freq = fmin(*inputs[kGenFreqIn] * params[0], sample_rate / 2);
    phase_inc = freq * 2.0 * M_PI * SR_cst;
}

void Generator::set_freq(double _freq)
{
    freq = _freq;
    phase_inc = freq * 2.0 * M_PI * SR_cst;
}
double Generator::get_phase()
{
    return phase;
}
void Generator::inc_phase ()
{
    
    if (*inputs[kGenGate] != gate)
    {
        gate = *inputs[kGenGate];
        if (gate == true)
            phase = 0.0;
    }

    phase += *inputs[kGenPhaseIn] + phase_inc;

    while (phase < 0.0)
        phase += 2 * M_PI;
        
    while (phase >= 2 * M_PI)
        phase -= 2 * M_PI;

}

}
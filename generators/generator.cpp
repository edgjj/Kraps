
#include <math.h>

#include "generator.hpp"

Generator::Generator (uint8_t type, uint8_t i, uint8_t o) : Processor (type, i + 3, o + 2)
{

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
    freq = fmin(*inputs[kGenFreqIn], sample_rate / 2);
    phase_inc = freq * 2.0 * M_PI * SR_cst;
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
        
    while (phase > 2 * M_PI)
        phase -= 2 * M_PI;

}

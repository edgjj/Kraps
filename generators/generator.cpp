
#include <math.h>

#include "generator.hpp"

Generator::Generator (uint8_t i, uint8_t o) : Processor (p_generator, i + 2, o + 2)
{

}

Generator::~Generator ()
{

}

void Generator::set_freq () 
{
    this->freq = fmin(get_sample(kGenFreqIn), sample_rate / 2);
    this->phase_inc = *inputs[kGenFreqIn] * 2.0 * M_PI * SR_cst;
}

void Generator::set_phase (double phase)
{
    this->phase = phase;
}

void Generator::inc_phase ()
{
    phase += phase_inc;
    while (phase >= 2 * M_PI)
        phase -= 2 * M_PI;
}

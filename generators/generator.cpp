
#include <math.h>

#include "generator.hpp"

void Generator::set_freq (double freq) 
{
    this->freq = fmin(freq, sample_rate / 2);
    this->phase_inc = this->freq * 2 * M_PI / sample_rate;
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

void Generator::set_SR (double SR) 
{
    sample_rate = SR;
    set_freq (freq);
}
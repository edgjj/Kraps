/* by Tale: https://www.kvraudio.com/forum/viewtopic.php?t=375517 & Martin Finke: http://martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/ */

#ifndef PBLEP_H
#define PBLEP_H

#include <stdint.h>


typedef enum pBLEP_mode{
    pblep_sine,
    pblep_triangle,
    pblep_saw,
    pblep_square
} pBLEP_mode;

typedef struct pBLEPosc {

    pBLEP_mode mode;
    float SR;

    float phase;
    float phase_inc;
    float freq;
    double last_out;

} pBLEPosc;

void pblep_setup(pBLEPosc* osc, float sample_rate);
void pblep_set_freq(pBLEPosc* osc, float freq);
void pblep_set_SR(pBLEPosc* osc, float SR);
void pblep_set_mode(pBLEPosc* osc, pBLEP_mode mode);

double pblep_get_sample(pBLEPosc* osc);
double pblep_get_naive(pBLEPosc* osc);
double pblep_step(pBLEPosc* osc, double t);

#endif
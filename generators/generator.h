#ifndef AGENERATOR_H
#define AGENERATOR_H
#include <stdint.h>

#include "../processor/processor.h"


typedef struct generator_t {
    processor_t base;

    float SR_cst;
    float phase;
    float phase_inc;
    float freq;
} generator_t;

void generator_init(generator_t*, double, double);
void generator_set_freq(generator_t*, double);
void generator_destroy(generator_t*);

#endif
#ifndef APROCESSOR_H
#define APROCESSOR_H
#include "../misc/misc.h"
#include <stdint.h>

typedef struct processor_t{

    void (*process) (processor_t*, void*);

    uint8_t is_bypassed;
    double sample_rate;
    double* params;
    uint8_t num_params;


    double amp;
    double outputs[2];

} processor_t;


void processor_init (processor_t*, double, uint8_t);
void processor_set_callback (processor_t*, void (*fcn)(processor_t*, void*));
void processor_set_amp (processor_t*, double);
void processor_set_bypassed (processor_t*);
uint8_t processor_is_bypassed (processor_t*);
void processor_destroy (processor_t*);

#endif
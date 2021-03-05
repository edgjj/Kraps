#include "processor.h"

void processor_init (processor_t* proc, double SR, uint8_t num_params){
    proc->is_bypassed = 0;
    proc->sample_rate = SR;
    proc->num_params = num_params;
    proc->params = malloc(proc->num_params * sizeof(double));
}
void processor_set_bypassed (processor_t* proc){
    proc->is_bypassed = !proc->is_bypassed;
}


void processor_set_callback (processor_t* proc, void (*fcn)(processor_t*, void*)){
    proc->process = fcn;
}

void processor_set_amp (processor_t* proc, double amp){
    proc->amp = amp;
}

uint8_t processor_is_bypassed (processor_t* proc){
    return proc->is_bypassed;
}

void processor_destroy (processor_t* proc){
    
    free(proc->params);
    proc->process = (void*) NULL;
}
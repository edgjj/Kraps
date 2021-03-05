#include <math.h>


#include "generator.h"

void generator_init(generator_t* gen, double SR, double np){
    processor_init(gen,SR,np);

    gen->SR_cst = 1.0/SR;
    gen->freq = 1.0;
    gen->phase = 0.0;
    gen->phase_inc = 0.0;

}
void generator_set_freq(generator_t* gen, double freq){
    gen->freq = fmin(freq, gen->base.sample_rate / 2);
    gen->phase_inc = gen->freq * 2 * M_PI / gen->base.sample_rate;
}

void generator_destroy(generator_t* gen){
    processor_destroy(gen);
}
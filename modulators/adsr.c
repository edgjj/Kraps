#include <math.h>
#include <stdio.h>

#include "adsr.h"

void adsr_setup(ADSR* env, float SR){

    env->SR = SR;
    env->state = adsr_ENV_IDLE;
    
    env->attack_step = 1.f;
    env->decay_step = 1.f;
    env->sustain_amp = 1.f;
    env->release_step = 1.f;
    env->phase = 0.f;
    
}

void adsr_gate_on(ADSR* env){
    env->state = adsr_ENV_ATT;
    env->phase = 0.f;
    env->gate = 1;
}
void adsr_gate_off(ADSR* env){
    env->state = adsr_ENV_REL;
    env->gate = 0;
}
float adsr_get_coeff(ADSR* env){
    float ret = env->phase;

    switch (env->state){
        case adsr_ENV_ATT:
            if (env->phase < 1.f && env->attack_step > 0.f){
                env->phase += env->attack_step;
            } else {
                env->phase = 1.f;
                env->state = adsr_ENV_DECAY;
            }
            break;
        case adsr_ENV_DECAY:
            if (env->phase > env->sustain_amp && env->decay_step > 0.f){
                env->phase -= env->decay_step;
            } else {
                env->phase = env->sustain_amp;
                env->state = adsr_ENV_SUSTAIN;
            }
            break;
        case adsr_ENV_SUSTAIN:
            break;
        case adsr_ENV_REL:
            if (env->phase > 0.f && env->release_step > 0.f){
                env->phase -= env->release_step;
            }
            else {
                env->phase = 0.f;
                env->state = adsr_ENV_IDLE;
            }
            break;
    }

    return ret;
}

void adsr_set_attack_time(ADSR* env, float time){
    if (time <= 0.f){
        env->attack_step = 0.f; return;
    }
    env->attack_step = 1.f/(env->SR*time);
}
void adsr_set_decay_time(ADSR* env, float time){
    if (time <= 0.f){
        env->decay_step = 0.f; return;
    }
    
    env->decay_step = (1.f-env->sustain_amp)/(env->SR*time);
}

// V to dB = 10^(V/20); dB to V = 20*log(dB);
void adsr_set_sustain_amp(ADSR* env, float amp){
    env->sustain_amp = pow(10,amp/20);
}
void adsr_set_release_time(ADSR* env, float time){
    if (time <= 0.f){
        env->release_step = 0.f; return;
    }
    env->release_step = env->sustain_amp/(env->SR*time);
   
}
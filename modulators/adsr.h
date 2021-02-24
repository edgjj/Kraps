#ifndef ADSR_h
#define ADSR_h

#include <stdint.h>

typedef enum adsr_Env_STATE {
    adsr_ENV_IDLE,
    adsr_ENV_ATT,
    adsr_ENV_DECAY,
    adsr_ENV_SUSTAIN,
    adsr_ENV_REL,
} adsr_Env_STATE;

typedef struct ADSR{
    adsr_Env_STATE state;
    float attack_step;
    float decay_step;
    float release_step;
    float sustain_amp;
    float phase;

    uint8_t gate;

    float SR;
    float step;
} ADSR;

void adsr_setup(ADSR* env, float SR);

void adsr_gate_on(ADSR* env);
void adsr_gate_off(ADSR* env);
float adsr_get_coeff(ADSR* env);

void adsr_set_attack_time(ADSR* env, float time);
void adsr_set_decay_time(ADSR* env, float time);
void adsr_set_sustain_amp(ADSR* env, float amp);
void adsr_set_release_time(ADSR* env, float time);


#endif
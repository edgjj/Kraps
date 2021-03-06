#include <cmath>

#include "adsr.hpp"


ADSR::ADSR () : Processor (p_modulator, 2, 1)
{
    
}


void ADSR::gate_on()
{
    state = adsr_ENV_ATT;
    pos = 0.0;
    gate = true;
}
void ADSR::gate_off()
{
    state = adsr_ENV_REL;
    gate = false;
}

void ADSR::process()
{
    float ret = pos;

    if (*inputs[kADSRGate] == true)
    {
        gate_on();
    }
    else
    {
        gate_off();
    }

    switch (state){
        case adsr_ENV_ATT:
            if (pos < 1.f && step[adsr_attack] > 0.f){
                pos += step[adsr_attack];
            } else {
                pos = 1.f;
                state = adsr_ENV_DECAY;
            }
            break;
        case adsr_ENV_DECAY:
            if (pos > sustain_amp && step[adsr_decay] > 0.f){
                pos -= step[adsr_decay];
            } else {
                pos = sustain_amp;
                state = adsr_ENV_SUSTAIN;
            }
            break;
        case adsr_ENV_SUSTAIN:
            break;
        case adsr_ENV_REL:
            if (pos > 0.f && step[adsr_release] > 0.f){
                pos -= step[adsr_release];
            }
            else {
                pos = 0.f;
                state = adsr_ENV_IDLE;
            }
            break;
    }

    *outputs[kADSRAudioOut] = *inputs[kADSRAudioIn] * ret;
    
}

void ADSR::process_params ()
{
    for (int i = 0; i < params.size(); i++)
    {
        if (i == adsr_sustain)
        {
            sustain_amp = pow(10, params[i] / 20.0);
            return;
        }

        if (params[i] <= 0.0)
        {
            step[i] = 0.0;
            continue;
        }
        step[i] = 1.0 / (sample_rate * params[i]);
    }
}


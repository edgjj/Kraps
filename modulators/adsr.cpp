#include <cmath>

#include "adsr.hpp"



ADSR::ADSR () : Processor (p_adsr, 1, 1)
{
    params = std::vector<double> (4, 0.0);
    params_constrainments = std::vector<std::pair <double, double >>(3, std::pair<double, double>(0.0, 20.0));
    params_constrainments.push_back(std::pair(-60, 0));
}


void ADSR::set_gate(bool g)
{
    if (g == true)
    {
        state = adsr_ENV_ATT;
        pos = 0.0;
    } else 
    {
        state = adsr_ENV_REL;
    }
    

    gate = g;
}


void ADSR::process_callback()
{
    double ret = pos;

    if (*inputs[kADSRGate] != gate)
    {
        set_gate(*inputs[kADSRGate]);
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

    *outputs[kADSRAudioOut] = ret;
    
}

void ADSR::process_params ()
{
    for (int i = 0; i < params.size(); i++)
    {
        if (i == adsr_sustain)
        {
            sustain_amp = pow(10, params[i] / 20.0);
            continue;
        }

        if (params[i] <= 0.0)
        {
            step[i] = 0.0;
            continue;
        }
        step[i] = 1.0 / (sample_rate * params[i]);
    }
}


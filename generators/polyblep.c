#include <math.h>

#include "../misc/misc.h"
#include "polyblep.h"

void pblep_setup(pBLEPosc* osc, float sample_rate){

    assert(sample_rate > 0);
    osc->mode = pblep_sine;

    osc->SR = sample_rate;
    osc->freq = 1.f;
    osc->last_out = 0.f;
    osc->phase = 0.f;
    osc->phase_inc = 0.f;

}

void pblep_set_freq(pBLEPosc* osc, float freq){

    osc->freq = fmin(freq, osc->SR/2);
    osc->phase_inc = osc->freq * 2 * M_PI / osc->SR;

}
double pblep_get_sample(pBLEPosc* osc){
    double out = 0.f;
    double t = osc->phase / (2.0*M_PI);
    out = pblep_get_naive(osc);
    if (osc->mode == pblep_sine){
        out = out;
    }      
    else if (osc->mode == pblep_saw) {
        out -= pblep_step(osc,t);
    } else {
        out += pblep_step(osc,t);
        out -= pblep_step(osc,fmod(t + 0.5, 1.0));
        if (osc->mode == pblep_triangle) {
            // Leaky integrator: y[n] = A * x[n] + (1 - A) * y[n-1]
            out = osc->phase_inc * out + (1 - osc->phase_inc) * osc->last_out;
            osc->last_out = out;
        }
    }
    


    osc->phase += osc->phase_inc;
    while (osc->phase >= 2*M_PI)
        osc->phase -= 2*M_PI;
        
    return out;
}

double pblep_get_naive(pBLEPosc* osc){
    double value;
    switch (osc->mode) {
        case pblep_sine:
            value = sin(osc->phase);
            break;
        case pblep_triangle:
            value = -1.0 + ( osc->phase / M_PI);
            value = 2.0 * (fabs(value) - 0.5);
            break;
        case pblep_saw:
            value = 1.0 - (osc->phase / M_PI);
            break;
        case pblep_square:
            if (osc->phase < M_PI) {
                value = 1.0;
            } else {
                value = -1.0;
            }
            break;
        
        default:
            break;
    }
    return value;
}

double pblep_step(pBLEPosc* osc, double t){

    double dt = osc->phase_inc / (2*M_PI);
    
    // 0 <= t < 1
    if (t < dt) {
        t /= dt;
        return t+t - t*t - 1.0;
    }
    // -1 < t < 0
    else if (t > 1.0 - dt) {
        t = (t - 1.0) / dt;
        return t*t + t+t + 1.0;
    }
    // 0 otherwise
    else return 0.f;

}
void pblep_set_SR(pBLEPosc* osc, float SR){
    osc->SR = SR;
    pblep_set_freq(osc,osc->freq);
}

void pblep_set_mode(pBLEPosc* osc, pBLEP_mode mode){
    osc->mode = mode;
}

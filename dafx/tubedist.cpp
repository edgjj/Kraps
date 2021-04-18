#include "tubedist.hpp"

namespace kraps
{
namespace dafx
{
TubeDist::TubeDist() : Processor(p_tube, 1, 1)
{
    params = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
    params_constrainments = { {0.0, 30.0}, {0.0, 20.0}, {0.0, 100.0}, {0.0, 100.0}, {0.0, 100.0}, {-60.0, 10.0} };
    process_params();
}
TubeDist::~TubeDist()
{
	;
}
void TubeDist::process_params()
{
    tC = (params[2] < 0.01 ? 0.01 : params[2]) * 2 + 1;
    bC = (params[3] + 0.166666667) * 6;
    pC = params[4] * 8;

    auto d2g = [](double dB) { return pow(10, dB / 20); };
    gain = d2g(params[0]);
    pre_gain = d2g(params[1]);
    out_gain = d2g(params[5]);
}

void TubeDist::process_callback()
{
    double in = *inputs[kDAFXAudioIn];
    double outsmp = in * pre_gain;
    if (outsmp < 0) outsmp = tanh((outsmp / bC) * gain) * bC;

    //scaled biased x^1.5
    outsmp += bC;
    outsmp = outsmp * sqrtf(fabs(outsmp)) - bC * sqrtf(fabs(bC)); // x^1.5 = in*(sqrt(abs(in))

    outsmp /= sqrtf(bC) * 1.5;

    if (outsmp >= 0) {
        outsmp *= gain;
        outsmp /= tC;
        outsmp = tanh(outsmp) * (pC + 1) - (outsmp / (sqrtf(outsmp * outsmp + 1))) * pC;
        outsmp *= tC;
    }

    outsmp *= out_gain;
    *outputs[kDAFXAudioOut] = outsmp;
}
}
}
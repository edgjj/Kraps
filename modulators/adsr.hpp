#ifndef ADSR_h
#define ADSR_h

#include <cstdint>
#include "../processor/processor.hpp"

namespace kraps {

enum adsr_params {
    adsr_attack,
    adsr_decay,
    adsr_release,
    adsr_sustain
};

enum kADSRInputs {
    kADSRGate
};

enum kADSROutputs {
    kADSRAudioOut
};

class ADSR : public Processor
{
public:
    ADSR ();

    ~ADSR () { ; }
protected:

    void set_gate (bool);


    void process_callback () override;
    void process_params () override;
    void recalculate_sr() override { ; }

private:
    enum adsr_state {
        adsr_ENV_IDLE,
        adsr_ENV_ATT,
        adsr_ENV_DECAY,
        adsr_ENV_SUSTAIN,
        adsr_ENV_REL,
    };


    bool gate = false;
    adsr_state state = adsr_ENV_IDLE;
    double pos = 0.0; 
    std::array <double, 3> step = { 0.0 };
    double sustain_amp = 0.0;
};

}

#endif
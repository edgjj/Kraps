#ifndef ADSR_h
#define ADSR_h

#include <cstdint>
#include "../processor/processor.hpp"

enum adsr_state {
    adsr_ENV_IDLE,
    adsr_ENV_ATT,
    adsr_ENV_DECAY,
    adsr_ENV_SUSTAIN,
    adsr_ENV_REL,
};

enum adsr_params {
    adsr_attack,
    adsr_decay,
    adsr_release,
    adsr_sustain
};
enum kADSRInputs {
    kADSRAudioIn,
    kADSRGate
};

enum kADSROutputs {
    kADSRAudioOut
};

class ADSR : public Processor
{
public:
    ADSR ();

    
    void process () override;
   


    void gate_on ();
    void gate_off ();



    ~ADSR () { ; }
protected:
    void process_params () override;
    void recalculate_sr() override { process_params(); }

private:
    bool gate = false;
    adsr_state state = adsr_ENV_IDLE;
    double pos = 0.0; 
    std::array <double, 3> step;
    double sustain_amp = 0.0;
};



#endif
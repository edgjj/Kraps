#ifndef ADSR_h
#define ADSR_h

#include <cstdint>
#include "../processor/processor.hpp"

typedef enum adsr_state {
    adsr_ENV_IDLE,
    adsr_ENV_ATT,
    adsr_ENV_DECAY,
    adsr_ENV_SUSTAIN,
    adsr_ENV_REL,
} adsr_Env_STATE;

typedef enum adsr_params {
    adsr_attack,
    adsr_decay,
    adsr_release,
    adsr_sustain
} adsr_params;

class ADSR : public Processor
{
public:
    ADSR (double);

    void process_params () override;
    void process () override;
    
    void gate_on () { gate = true; }
    void gate_off () { gate = false; }

    ~ADSR () { ; }
private:
    bool gate = false;
    adsr_state state = adsr_ENV_IDLE;
    double pos = 0.0; 
    std::array <double, 3> step;
    double sustain_amp = 0.0;
};



#endif
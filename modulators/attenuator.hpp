#ifndef ATTENUATOR_H
#define ATTENUATOR_H
#include "../processor/processor.hpp"

enum kAttenInputs {
    kAttenIn,
    kAttenMod
};

enum kAttenOutputs {
    kAttenOut
};

class Attenuator : public Processor
{
public:
    Attenuator () : Processor (p_misc, 2, 1)
    {
        ;
    }
    ~Attenuator() { ; }
protected:
    void process_callback () override
    {
        *outputs[kAttenOut] = *inputs[kAttenIn] * *inputs[kAttenMod];
    }
    void process_params () override { ; }
    void recalculate_sr () override { ; }

private:
    
};


#endif
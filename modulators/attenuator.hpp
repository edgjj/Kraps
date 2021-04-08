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
    Attenuator () : Processor (p_atten, 2, 1)
    {
        params = std::vector<double>(1, 0.0);
        params_constrainments = std::vector<std::pair <double, double >>(1, std::pair<double, double>(0.0, 1.0));
    }
    ~Attenuator() { ; }
protected:
    void process_callback () override
    {
        *outputs[kAttenOut] = *inputs[kAttenIn] * params[0];
        if (inputs[kAttenMod]->src->val != 0.0)
            *outputs[kAttenOut] = *outputs[kAttenOut] *  *inputs[kAttenMod];
    }
    void process_params () override { ; }
    void recalculate_sr () override { ; }

private:
    
};


#endif
#ifndef ATTENUATOR_H
#define ATTENUATOR_H
#include "../processor/processor.hpp"

namespace kraps {

enum kAttenInputs {
    kAttenIn1,
    kAttenMod1,
    kAttenIn2,
    kAttenMod2,
    kAttenIn3,
    kAttenMod3,
    kAttenIn4,
    kAttenMod4,
};

enum kAttenOutputs {
    kAttenOut1,
    kAttenOut2,
    kAttenOut3,
    kAttenOut4
};

class Attenuator : public Processor
{
public:
    Attenuator () : Processor (p_atten, 8, 4)
    {

    }
    ~Attenuator() { ; }
protected:
    void process_callback () override
    { 
        for (int i = 0; i < outputs.size(); i++)
            *outputs[i] = *inputs[i * 2] * *inputs[i * 2 + 1];
    }
    void process_params () override { ; }
    void recalculate_sr () override { ; }

private:
    
};

class Summer : public Processor
{
public:
    Summer() : Processor(p_summer, 8, 4)
    {

    }
    ~Summer() { ; }
protected:
    void process_callback() override
    {
        for (int i = 0; i < outputs.size(); i++)
            *outputs[i] = *inputs[i * 2] + *inputs[i * 2 + 1];
    }
    void process_params() override { ; }
    void recalculate_sr() override { ; }

private:

};

}
#endif
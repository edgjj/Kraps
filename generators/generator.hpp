#ifndef AGENERATOR_H
#define AGENERATOR_H
#include "../processor/processor.hpp"
namespace kraps {




enum kGenInputs 
{
    kGenFreqIn,
    kGenPhaseIn,
    kGenGate
};

enum kGenOutputs 
{
    kGenAudioOut,
    kGenPhaseOut
};

class Generator : public Processor 
{
public:
    Generator(uint8_t type,uint8_t i, uint8_t o);

    void set_freq ();
    void set_phase (double);
    void inc_phase ();

    ~Generator ();

protected:
    void recalculate_sr () override 
    {
        SR_cst = 1.0 / get_SR();  
    }   
    double phase = 0.0;
    bool gate = 0.0;
    double freq = 0.0;
    double SR_cst = 0.0;
private:
    double phase_inc = 0.0;

};

}
#endif
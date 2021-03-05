#ifndef AGENERATOR_H
#define AGENERATOR_H
#include "../processor/processor.hpp"

class Generator : public Processor 
{
public:
    Generator(double SR) : Processor(p_generator, SR), SR_cst (1.0 / SR) { ; }

    void set_freq (double);
    void set_phase (double);
    void set_SR (double);
    
    void inc_phase ();
    
    ~Generator () { ; }

protected:
    double phase;
    double SR_cst;
    double freq;
private:
    double phase_inc;
};

#endif
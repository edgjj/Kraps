#ifndef BASICG_H
#define BASICG_H
#include <array>
#include "../Kraps/generators/generator.hpp"

#define NUM_OCTAVES 11



class BasicG : public Generator {
public:

    BasicG() : Generator(p_misc, 0, 0) { ; }

    ~BasicG() { ; } 

protected:
    void process_callback() override
    {
        set_freq();

        *outputs[kGenAudioOut] = sin(phase);
        *outputs[kGenPhaseOut] = phase;

        inc_phase();
    }
    void process_params() override { ; }
private:

};

#endif 
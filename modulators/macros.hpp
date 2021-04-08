#ifndef MACROS_H
#define MACROS_H
#include "../processor/processor.hpp"

enum kMacroOutputs {
    kMacroOut1,
    kMacroOut2,
    kMacroOut3,
    kMacroOut4,
    kMacroOut5,
    kMacroOut6,
    kMacroOut7,
    kMacroOut8,
    kMacroCnt = kMacroOut8
};

class Macro : public Processor
{
public:
    Macro() : Processor(p_macro, 0, 8)
    {
        params = std::vector<double>(8, 0.0);
        params_constrainments = std::vector<std::pair <double, double >>(1, std::pair<double, double>(0.0, 1.0));
    }
    ~Macro() { ; }
protected:
    void process_callback() override
    {
        for (int i = 0; i < kMacroCnt; i++)
            *outputs[i] = params[i];
    }
    void process_params() override { ; }
    void recalculate_sr() override { ; }

private:

};

#endif
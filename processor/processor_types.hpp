#ifndef PROCESSOR_DEFS_H
#define PROCESSOR_DEFS_H

#include <map>
#include <string>

typedef enum ProcessorTypes 
{
    p_wt,
    p_lfo,
    p_adsr,
    p_filter,
    p_dafx,
    p_atten,
    p_macro,
    p_misc,
    p_count = p_macro
} ProcessorTypes;

static std::map <ProcessorTypes, std::string> ProcessorTypesDesc {
    { p_wt, "Wavetable" },
    { p_lfo, "LFO" },
    { p_adsr, "ADSR Env." },
    { p_filter, "Filter" },
    { p_dafx, "FX" },
    { p_atten, "Attenuator"},
    { p_macro, "Macro-8"},
    { p_misc, "Miscellaneous"}
};

#endif
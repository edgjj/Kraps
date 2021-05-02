#ifndef PROCESSOR_DEFS_H
#define PROCESSOR_DEFS_H

#include <map>
#include <string>

namespace kraps{

typedef enum ProcessorTypes 
{
    p_wt = 1,
    p_lfo,
    p_adsr,
    p_filter,
    p_tube,
    p_atten,
    p_macro,
    p_delay,
    p_summer,
    p_basic_gen,
    p_notemgr = 240,
    p_output,
    p_misc,
    p_count = p_basic_gen
} ProcessorTypes;


static std::map <ProcessorTypes, std::string> ProcessorTypesDesc {
    { p_wt, "Wavetable" },
    { p_lfo, "LFO" },
    { p_adsr, "ADSR Env." },
    { p_filter, "Filter" },
    { p_tube, "Tube Dist." },
    { p_atten, "Attenuator Bank"},
    { p_basic_gen, "Basic Generator"},
    { p_summer, "Summer Bank"},
    { p_macro, "Macro-8"},
    { p_delay, "Delay"},
    { p_notemgr, "Note Manager"},
    { p_output, "Output Node"},
    { p_misc, "Miscellaneous"}
};

static std::map<ProcessorTypes, std::vector <std::string>> ProcessorParametersDesc = 
{
    { p_wt, {"Freq. Multiplier", "WT. Pos"}},
    { p_basic_gen, {"Frequency Multiplier"}},
    { p_lfo, {"Frequency", "Envelope Mode"}},
    { p_adsr, {"Attack", "Decay", "Release", "Sustain Gain"}},
    { p_delay, {"Time", "Feedback", "Dry/Wet"}},
    { p_filter, {"Frequency", "Q", "Reserved"}},
    { p_tube, {"Gain", "Pre. Gain", "Top", "Bot", "Peak", "Output Gain"}},
    { p_notemgr, {"A3 Tuning"}}
};



}

#endif
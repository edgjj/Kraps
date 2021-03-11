#ifndef PROCESSOR_DEFS_H
#define PROCESSOR_DEFS_H

#include <map>
typedef enum ProcessorTypes 
{
    p_generator,
    p_modulator,
    p_filter,
    p_dafx
} ProcessorTypes;

static std::map <ProcessorTypes, std::string> ProcessorTypesDesc {
    { p_generator, "Generator" },
    { p_modulator, "Modulator" },
    { p_filter, "Filter" },
    { p_dafx, "FX" },
};

#endif
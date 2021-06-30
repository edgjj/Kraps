/* Copyright 2021 Yegor Suslin
 *
 * This file is part of Kraps library.
 *
 * Kraps is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kraps is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kraps.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KRAPS_PROCESSOR_DEFS_H
#define KRAPS_PROCESSOR_DEFS_H

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
    p_sampler,
    p_decomposer,
    p_pulverizer,
    p_compressor,
    p_notemgr = 240,
    p_output,
    p_count = p_compressor
} ProcessorTypes;

// i definitely think there should be better solution instead of static descriptions

static std::map <ProcessorTypes, std::string> ProcessorTypesDesc {
    { p_wt, "Wavetable" },
    { p_lfo, "LFO" },
    { p_adsr, "ADSR Env." },
    { p_filter, "Filter" },
    { p_tube, "Tube Dist." },
    { p_atten, "Attenuator Bank"},
    { p_sampler, "Sampler"},
    { p_summer, "Summer Bank"},
    { p_macro, "Macro-8"},
    { p_delay, "Delay"},
    { p_notemgr, "Note Manager"},
    { p_output, "Output Node"},
    { p_decomposer, "Decomposer"},
    { p_pulverizer, "Pulverizer"},
    { p_compressor, "Compressor"}
};

static std::map<ProcessorTypes, std::vector <std::string>> ProcessorParametersDesc = 
{
    { p_wt, {"Freq. Multiplier", "Freq. Shift", "WT. Pos"}},
    { p_sampler, {"Root Freq.", "Sample Start", "Loop Mode"}},
    { p_lfo, {"Frequency", "Envelope Mode", "Numerator", "Denominator"}},
    { p_adsr, {"Attack", "Decay", "Release", "Sustain Gain"}},
    { p_delay, {"Time", "Feedback", "Dry/Wet"}},
    { p_filter, {"Frequency", "Q", "Reserved"}},
    { p_tube, {"Gain", "Pre. Gain", "Top", "Bot", "Peak", "Output Gain"}},
    { p_notemgr, {"Mono Mode", "Legato", "Always Porta", "Porta Time"}},
    { p_pulverizer, { "1st Frequency", "1st Q", "1st Amount", "2nd Frequency", "2nd Q", "2nd Amount"}},
    { p_compressor, { "Threshold", "Ratio", "Knee", "Attack", "Release", "Out. Gain"}}

};



}

#endif
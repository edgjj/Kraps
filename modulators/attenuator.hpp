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
        // needs to be reworked for simd
        for (int i = 0; i < outputs.size(); i++)
        {
            float8 i1 = *inputs[i * 2];
            float8 i2 = *inputs[i * 2 + 1];
            *outputs[i] = i1 * i2;
        }
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
        {
            float8 i1 = *inputs[i * 2];
            float8 i2 = *inputs[i * 2 + 1];
            *outputs[i] = i1 + i2;
        }
    }
    void process_params() override { ; }
    void recalculate_sr() override { ; }

private:

};

}
#endif
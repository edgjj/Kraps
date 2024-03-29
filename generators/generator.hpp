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

    void set_freq (double);
    void set_freq ();
    void set_phase (double);
    double get_phase();
    void inc_phase ();

    virtual ~Generator ();

protected:
    void recalculate_sr () override 
    {
        SR_cst = 1.0 / get_SR();  
    }   
    double phase = 0.0;
    bool gate = 0.0;
    double freq = 0.0;
    double SR_cst = 0.0;
    double phase_inc = 0.0;
private:
    

};

}
#endif
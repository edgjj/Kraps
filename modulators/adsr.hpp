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

#ifndef ADSR_h
#define ADSR_h

#include "../processor/processor.hpp"

namespace kraps {

enum adsr_params {
    adsr_attack,
    adsr_decay,
    adsr_release,
    adsr_sustain
};

enum kADSRInputs {
    kADSRGate
};

enum kADSROutputs {
    kADSRAudioOut
};

class ADSR : public Processor
{
public:
    ADSR ();
    float8 get_position();

    ~ADSR () { ; }
protected:

    void set_gate ();

    void process_callback () override;
    void process_params () override;
    void recalculate_sr() override { ; }

private:
    enum adsr_state {
        adsr_ENV_IDLE,
        adsr_ENV_ATT,
        adsr_ENV_DECAY,
        adsr_ENV_SUSTAIN,
        adsr_ENV_REL,
    };


    float8 gate = false;
    float8 state = adsr_ENV_IDLE;
    float8 pos = 0.0; 

    std::array <float8, 3> step = { 0.0 };
    double sustain_amp = 0.0;
};

}

#endif
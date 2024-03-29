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

#ifndef BASICG_H
#define BASICG_H
#include <array>
#include "../Kraps/generators/generator.hpp"



namespace kraps{

class BasicG : public Generator {
public:

    BasicG() : Generator(p_basic_gen, 0, 0) { ; }

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

}
#endif 
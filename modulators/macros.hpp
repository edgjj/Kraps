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

#ifndef MACROS_H
#define MACROS_H
#include "../processor/processor.hpp"

namespace kraps {

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
        params_constrainments = std::vector<std::pair <double, double >>(8, std::pair<double, double>(0.0, 1.0));
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

}

#endif
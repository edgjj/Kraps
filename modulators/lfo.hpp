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

#ifndef KRAPS_LFO_H
#define KRAPS_LFO_H
#include <map>
#include "../generators/generator.hpp"
namespace kraps {

enum kLFOInputs 
{
    kLFOFreqIn,
    kLFOPhaseIn,
    kLFOGate
};

enum kLFOOutputs 
{
    kLFOAudioOut,
    kLFOPhaseOut
};

class LFO : public Generator
{
public:
    LFO();

    std::pair<std::vector<FVec2>, std::vector<double>> get_points();
    void add_point (FVec2);
    void move_point (int, FVec2);
    void set_tension (int, double);
    void remove_point (int);
    
    const nlohmann::json get_serialize_obj() override;
    void set_serialize(const nlohmann::json&) override;
    float get_interp(float x);

    ~LFO() { ; }
protected:
    void inc_phase();
    void process_callback () override;
    void process_params() override;
private:
    inline double sigmoid (double x, double k);
    

    double param_freq = 0.0;
    double freq_ratio = 0.0;


    bool is_env = false;
    double phase_const = 1 / (2 * M_PI);

    std::vector<FVec2> points;
    std::vector <double> tension; // == N + 2 / 2
};

}
#endif
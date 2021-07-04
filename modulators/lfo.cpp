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

#include <cmath>

#include "lfo.hpp"


/* https://dhemery.github.io/DHE-Modules/technical/sigmoid/ */
namespace kraps {

LFO::LFO () : Generator (p_lfo, 0, 0),
    points({ { 0.0, 0.0 }, {0.5, 1.0}, { 1.0, 0.0 } }),
    tension ({ 0.0, 0.0 })
{
    pt = kraps::parameter::pt::ParameterTable(
        new parameter::Parameter<float>("freq_mult", 1, 1, 0, 64),
         new parameter::Parameter<bool>("is_envelope", false, false, false, true),
         new parameter::Parameter<int>("numerator", 1, 1, 1, 32.0),
         new parameter::Parameter<int>("denominator", 4, 4, 1, 256.0)
        );

}

void LFO::add_point(Vec2 pos)
{
    set_lock();
    for (int i = 0; i < points.size() - 1; i++)
    {
        Vec2 p1, p2;
        p1 = points[i];
        p2 = points[i+1];
        if (pos.x > p1.x && pos.x < p2.x)
        {
            points.insert (points.begin () + i + 1, pos);\
            if (tension.size() == 0)
            {
                tension.insert (tension.begin () + i, 0.0);
                tension.insert (tension.begin () + i + 1, 0.0);
            }       
            else tension.insert (tension.begin () + i + 1, 0.0); 

            break;
        }
    }
    set_unlock();
}

void LFO::process_params()
{
    param_freq = pt.get_raw_value ("freq_mult");
    is_env = pt.get_raw_value("is_envelope");    
    float d = pt.get_raw_value("denominator"), n = pt.get_raw_value("numerator");
    freq_ratio = d / n;
}

std::pair<std::vector<Vec2>, std::vector<double>> LFO::get_points()
{
    return std::make_pair(points,tension);
}


void LFO::move_point (int i, Vec2 pos)
{
    set_lock();
    if (i == 0 || i == points.size() - 1){
        points.front().y    = pos.y;
        points.back().y     = pos.y;
    }
    else points[i] = pos;
    set_unlock();
}

void LFO::set_tension (int _i, double _tension)
{
    set_lock();
    if (points.size() == 2)
        return;
    int8_t sign         = std::signbit (_tension) * 2 - 1;
    this->tension[_i]    = fabs (_tension) > 0.9999 ? sign * 0.9999 : _tension;
    set_unlock();

}

void LFO::remove_point (int i)
{
    set_lock();
    if (points.size() == 2)
        return;

    if (i == 0 || i >= points.size() - 1)
        return;
    points.erase (points.begin() + i);
    tension.erase (tension.begin() + i - 1);
    set_unlock();
}

float LFO::get_interp (float x){
    Vec2    p1, p2;
    double  y_diff;
    int8_t  sign;
    
    for (int i = 0; i < points.size() - 1; i++)
    {
        p1 = points[i]; 
        p2 = points[i+1];
        if (x >= p1.x && x < p2.x)
        {
            if (points.size() == 2)
            {
                return p1.y;
            }
            y_diff  = p2.y - p1.y;
            sign    = std::signbit (y_diff) * -2 + 1;

            return sigmoid ( ( (x - p1.x) / (p2.x - p1.x) ) , tension [i] * sign ) * y_diff + p1.y;
        }
    } 
    return points.back().y;
};

void LFO::inc_phase()
{

    float8 cmp = *inputs[kGenGate] != gate;

    if (movemask(cmp) != 0)
    {
        float8 cmp_gate = andnot(gate, *inputs[kGenGate]) == float8(1.0f);
        phase = blend(phase, float8(0), cmp_gate);
        gate = *inputs[kGenGate];
    }

    phase += *inputs[kGenPhaseIn] + phase_inc;
    float8 mpi2 = 2 * M_PI;

    if (is_env)
    {
        phase = smin(phase, mpi2);
    }
    else
    { 
        while (movemask(phase < float8(0.f)) != 0)
            phase = blend(phase, phase + mpi2, phase < float8(0.f));

        while (movemask(phase >= mpi2) != 0)
            phase = blend(phase, phase - mpi2, phase >= mpi2);
    }
    

}

void LFO::process_callback ()
{
    double lfo_freq = inputs[kGenFreqIn]->src->id != -1 ? *inputs[kGenFreqIn] * float8 (freq_ratio) : param_freq;
        
    set_freq(lfo_freq);
    float8 phases = phase * float8(phase_const);

    // initial support
    float data[8];
    phases.storeu(data);

#pragma loop(hint_parallel(8))
    for (int i = 0; i < 8; i++)
        data[i] = get_interp(data[i]);


    *outputs[kLFOAudioOut] = phases.loadu(data);
    *outputs[kLFOPhaseOut] = phase;

    inc_phase();
}

const nlohmann::json LFO::get_serialize_obj()
{
    set_lock();
    nlohmann::json o;
    o["tension"] = tension;

    std::vector <std::array<double, 2>> pts_conv;

    for (auto& i : points)
        pts_conv.push_back({ i.x, i.y });

    o["points"] = pts_conv;

    set_unlock();

    o.update(Processor::get_serialize_obj());

    return o;
}

void LFO::set_serialize(const nlohmann::json& obj)
{
    Processor::set_serialize(obj);

    set_lock();

    if (obj.find("tension") != obj.end())
        obj["tension"].get_to(tension);

    if (obj.find("points") != obj.end())
    {
        std::vector <std::array<double, 2>> pts_conv;
        std::vector <Vec2> pts_conv_2;
        obj["points"].get_to(pts_conv);
        for (auto& i : pts_conv)
            pts_conv_2.push_back({ i[0], i[1] });

        points = pts_conv_2;
    }

    set_unlock();
}

inline double LFO::sigmoid(double x, double k) // k in [ -0.9999; 0.9999 ]
{
    return (x - x * k) / (k - fabs (x) * 2 * k + 1);
}

}
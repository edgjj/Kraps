#include <cmath>

#include "lfo.hpp"


/* https://dhemery.github.io/DHE-Modules/technical/sigmoid/ */
namespace kraps {

LFO::LFO () : Generator (p_lfo, 0, 0),
    points({ { 0.0, 0.0 }, {0.5, 1.0}, { 1.0, 0.0 } }),
    tension ({ 0.0, 0.0 })
{
    params.push_back(0.0);
    params.push_back(1.0);
    params.push_back(4.0);
    params_constrainments.push_back(std::pair(0.0, 1.0));
    params_constrainments.push_back(std::pair(1.0, 32.0));
    params_constrainments.push_back(std::pair(1.0, 256.0));
}

void LFO::add_point(Vec2 pos)
{
    WAIT_LOCK
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

}

void LFO::process_params()
{
    param_freq = params[0];
    is_env = params[1];    
    freq_ratio = params[3] / params[2];
}

std::pair<std::vector<Vec2>, std::vector<double>> LFO::get_points()
{
    return std::make_pair(points,tension);
}


void LFO::move_point (int i, Vec2 pos)
{
    WAIT_LOCK
    if (i == 0 || i == points.size() - 1){
        points.front().y    = pos.y;
        points.back().y     = pos.y;
    }
    else points[i] = pos;

}

void LFO::set_tension (int i, double tension)
{
    WAIT_LOCK
    if (points.size() == 2)
        return;
    int8_t sign         = std::signbit (tension) * 2 - 1;
    this->tension[i]    = fabs (tension) > 0.9999 ? sign * 0.9999 : tension;
}

void LFO::remove_point (int i)
{
    WAIT_LOCK
    if (points.size() == 2)
        return;

    if (i == 0 || i >= points.size() - 1)
        return;
    points.erase (points.begin() + i);
    tension.erase (tension.begin() + i - 1);

}

double LFO::get_interp (double x){
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

    if (*inputs[kGenGate] != gate)
    {
        gate = *inputs[kGenGate];
        if (gate == true)
            phase = 0.0;
    }

    phase += *inputs[kGenPhaseIn] + phase_inc;

    if (is_env)
    {
        phase = fmin(phase, 2 * M_PI);
    }
    else
    {
        while (phase < 0.0)
            phase += 2 * M_PI;

        while (phase >= 2 * M_PI)
            phase -= 2 * M_PI;
    }
    

}

void LFO::process_callback ()
{
    double freq = inputs[kGenFreqIn]->src->id != -1 ? *inputs[kGenFreqIn] * freq_ratio : param_freq;
        
    set_freq(freq);

    *outputs[kLFOAudioOut] = get_interp( phase * phase_const );
    *outputs[kLFOPhaseOut] = phase;

    inc_phase();
}

nlohmann::json LFO::get_serialize_obj()
{
    nlohmann::json o;
    o["tension"] = tension;
    o["is_env"] = is_env;

    std::vector <std::array<double, 2>> pts_conv;

    for (auto& i : points)
        pts_conv.push_back({ i.x, i.y });

    o["points"] = pts_conv;

    o.update(Processor::get_serialize_obj());

    return o;
}

void LFO::set_serialize(nlohmann::json obj)
{
    Processor::set_serialize(obj);
    if (obj.find("tension") != obj.end())
        obj["tension"].get_to(tension);

    if (obj.find("is_env") != obj.end())
        obj["is_env"].get_to(is_env);

    if (obj.find("points") != obj.end())
    {
        std::vector <std::array<double, 2>> pts_conv;
        std::vector <Vec2> pts_conv_2;
        obj["points"].get_to(pts_conv);
        for (auto& i : pts_conv)
            pts_conv_2.push_back({ i[0], i[1] });

        points = pts_conv_2;
    }
}

inline double LFO::sigmoid(double x, double k) // k in [ -0.9999; 0.9999 ]
{
    return (x - x * k) / (k - fabs (x) * 2 * k + 1);
}

}
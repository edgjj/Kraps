#include <cmath>

#include "lfo.hpp"


/* https://dhemery.github.io/DHE-Modules/technical/sigmoid/ */
namespace kraps {

LFO::LFO () : Generator (p_lfo, 0, 0),
    points({ { 0.0, 0.0 }, {0.5, 1.0}, { 1.0, 0.0 } }),
    tension ({ 0.0, 0.0 })
{

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

void LFO::move_point (int i, Vec2 pos)
{
    WAIT_LOCK
    if (i == 0){
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
    tension.erase (tension.begin() + i);
    
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
    return 0.0;
};

void LFO::process_callback ()
{
    set_freq();

    *outputs[kLFOAudioOut] = get_interp( phase * phase_const );
    *outputs[kLFOPhaseOut] = phase;

    inc_phase();
}

inline double LFO::sigmoid(double x, double k) // k in [ -0.9999; 0.9999 ]
{
    return (x - x * k) / (k - fabs (x) * 2 * k + 1);
}

}
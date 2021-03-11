#include <cmath>

#include "lfo.hpp"


/* https://dhemery.github.io/DHE-Modules/technical/sigmoid/ */

LFO::LFO () : Generator (0, 0),
    points ({ { 0.0, 0.0 }, { 1.0, 0.0 } }),
    tension ({})
{
    phase_const = lookup.size() / ( 2.0 * M_PI );
}

void LFO::add_point(Vec2 pos)
{
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
    preprocess();
    // add point

    // recalc phase constant = this->points.size() / ( 2.0 * M_PI );
}

void LFO::move_point (int i, Vec2 pos)
{
    if (i == 0){
        points.front().y = pos.y;
        points.back().y = pos.y;
    }
    else points[i] = pos;

    preprocess();
}

void LFO::set_tension (int i, double tension)
{
    if (points.size() == 2)
        return;
    int8_t sign = std::signbit (tension) * 2 - 1;
    this->tension[i] = fabs (tension) > 0.9999 ? sign * 0.9999 : tension;
    preprocess();
}

void LFO::remove_point (int i)
{
    if (points.size() == 2)
        return;
    points.erase (points.begin() + i);
    tension.erase (tension.begin() + i - 1);
    tension.erase (tension.begin() + i);
    preprocess();
}

void LFO::preprocess()
{

    auto get_interp = [&] (double x){
        Vec2 p1, p2;
        for (int i = 0; i < points.size() - 1; i++)
        {
            p1 = points[i]; p2 = points[i+1];
            if (x >= p1.x && x < p2.x)
            {
                if (points.size() == 2)
                {
                    return p1.y;
                }
                double y_diff = p2.y - p1.y;
                int8_t sign = std::signbit (y_diff) * -2 + 1;

                return sigmoid ( ( (x - p1.x) / (p2.x - p1.x) ) * 1, tension [i] * sign ) * y_diff + p1.y;
            }
        } 
        return 0.0;
    };

    for (int i = 0; i < lookup.size(); i++){
	    lookup[i] = ( get_interp ( (double) i / lookup.size() ) );
	}

}

void LFO::process_callback ()
{
    set_freq();


    // 2 * M_PI maps to points vector size;
    double phase_cvt = phase_const * phase;
    int32_t pos_int = phase_cvt; 
    double pos_frac = phase_cvt - pos_int;

    *outputs[kLFOAudioOut] = lookup[pos_int + 1] * pos_frac + lookup[pos_int] * (1 - pos_frac);
    *outputs[kLFOPhaseOut] = phase;

    inc_phase();
}

double LFO::sigmoid(double x, double k) // k in [ -0.9999; 0.9999 ]
{
    return (x - x * k) / (k - fabs (x) * 2 * k + 1);
}
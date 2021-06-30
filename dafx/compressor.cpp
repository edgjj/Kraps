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

#include "compressor.hpp"

namespace kraps
{
namespace dafx
{
Compressor::Compressor() : Processor (p_compressor, 1, 1)
{
    pt = kraps::parameter::pt::ParameterTable(
        { new parameter::Parameter<float>("threshold", 0, 0, -60.0, 0),
        new parameter::Parameter<float>("ratio", 1, 1, 0, 1.0),
        new parameter::Parameter<float>("knee", 0, 0, 0, 1.0),
        new parameter::Parameter<float>("attack", 0.05, 0.05, 0.001, 0.5),
        new parameter::Parameter<float>("release", 0.01, 0.01, 0.001 , 2.0),
        new parameter::Parameter<float>("out_gain", 0, 0, -30, 30.0),
        });

    process_params();

    io_description[0] =
    {
        {kDAFXAudioIn, "AUDIO", "Just audio input."}
    };

    io_description[1] =
    {
        {kDAFXAudioOut, "AUDIO", "Output for processed signal."}
    };
}

Compressor::~Compressor()
{

}

void Compressor::process_params()
{
    threshold = ln256_ps(pow256_ps(float8(10), pt.get_raw_value("threshold") / float8(20)));
    ratio = pt.get_raw_value("ratio");
    knee = pt.get_raw_value("knee");

    out_gain = pow256_ps(float8(10), pt.get_raw_value("out_gain") / float8(20));

    follower.setup(sample_rate, pt.get_raw_value("attack"), pt.get_raw_value("release"));

}

void Compressor::recalculate_sr()
{
    follower.setup(sample_rate, pt.get_raw_value("attack"), pt.get_raw_value("release"));
}
void Compressor::process_callback() 
{


    float8 in = *inputs[kDAFXAudioIn];
    float8 env = follower.process (in); 

    env = float8 ( ln256_ps(env) ) - threshold;
    
    float8 pre = env;

    env = env + float8 ( exp256_ps(env) ) * float8(-0.02);
    env = ssqrt (env * env + knee);

    pre += env * 0.88 + ftanh(env) * 1.1;
    pre *= 0.41;
    
    float8 g = ( 0 - smax(pre, 0.0) ) * ratio;
    g = exp256_ps(g);


    *outputs[kDAFXAudioOut] = g * in * out_gain;

}

}
}
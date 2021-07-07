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

#include "delay.hpp"

namespace kraps
{
namespace dafx
{
Delay::Delay() : Processor (p_delay, 2, 1)
{
	pt = kraps::parameter::pt::ParameterTable(
		{ new parameter::Parameter<float>("time", 0.02, 0.02, 0.001, 0.5),
		 new parameter::Parameter<float>("feedback", 0.7, 0.7, 0.0, 100.0),
		 new parameter::Parameter<float>("drywet", 0.5, 0.5, -0, 100.0),
		});


	io_description[0] =
	{
		{kDelayAudioIn, "AUDIO", "Just audio input."},
		{kDelayTimeIn, "TIME", "Time modulation input."}
	};

	io_description[1] =
	{
		{kDelayAudioIn, "AUDIO", "Output for processed signal."}
	};

}

Delay::~Delay()
{
	;
}
void Delay::recalculate_sr()
{
	dly_line.set_sample_rate(sample_rate);
	smoother.set_sample_rate(sample_rate);
}

void Delay::process_params()
{
	time = pt.get_raw_value("time");

	float8 sm_time = float8ops::clamp(*inputs[kDelayTimeIn] * 0.5 + time, 1 / sample_rate, 0.5);
	smoother.set_target(sm_time);
	smoother.set_time_cst(sm_time);

	feedback = pt.get_raw_value("feedback") / 100.0f;
	drywet = pt.get_raw_value("drywet") / 100.0f;
}
void Delay::process_callback()
{
	using namespace float8ops;
	float8 fbsmp = dly_line.get_interp(smoother.get_next_value());
	
	fbsmp = blend(fbsmp, float8(0), s_isnan(fbsmp));

	float8 in = *inputs[kDelayAudioIn];

	dly_line.push(in + fbsmp * feedback);
	float8 out = in + fbsmp * drywet;

	*outputs[kDelayAudioOut] = out;
}
}
}
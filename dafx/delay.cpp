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
	params = { 0.02, 0.7, 0.5 };
	params_constrainments = { {0.001, 0.5}, {0, 1}, {0, 1} };

	io_description[0] =
	{
		{kDelayAudioIn, "AUDIO", "Just audio input."},
		{kDelayTimeIn, "TIME", "Time modulation input."}
	};

	io_description[1] =
	{
		{kDelayAudioIn, "AUDIO", "Output for processed signal."}
	};


	smoother = std::make_unique <misc::LinearSmoother> (smoothed_time);
}

Delay::~Delay()
{
	;
}
void Delay::recalculate_sr()
{
	dly_line.set_sample_rate(sample_rate);
	smoother->set_sample_rate(sample_rate);
}

void Delay::process_params()
{
	param_time = params[0];
}
void Delay::process_callback()
{
	smoothed_time = fmax (fmin(*inputs[kDelayTimeIn] + param_time, 0.5), 0.001);
	float8 fbsmp = dly_line.get_interp(smoother->get_smoothed_value());
	
	fbsmp = blend(fbsmp, float8(0), s_isnan(fbsmp));

	float8 in = *inputs[kDelayAudioIn];

	dly_line.push(in + fbsmp * float8 (params[1]));
	float8 out = in + fbsmp * float8 (params[2]);

	*outputs[kDelayAudioOut] = out;
}
}
}
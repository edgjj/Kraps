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

#include "linear_smoother.hpp"
#include <cmath>
namespace kraps
{
namespace misc
{

LinearSmoother::LinearSmoother()
{

}

void LinearSmoother::set_sample_rate(float _sample_rate)
{
	sample_rate = _sample_rate; 
	sr_cst = 1.0 / sample_rate;
}

LinearSmoother::~LinearSmoother()
{

}

void LinearSmoother::set_time_cst(const float8& v)
{
	time_cst = v;
	step = (target - prev_value) * sr_cst / time_cst;
}

void LinearSmoother::set_target(const float8& v)
{
	using namespace float8ops;
	float8 mask = v != target;
	
	prev_value = blend (prev_value, target, mask);
	target = blend (target, v, mask);

	step = (target - prev_value) * sr_cst / time_cst;
}

float8 LinearSmoother::get_next_value()
{
	if (sample_rate == 0.0)
		return 0.0;

	step = float8ops::blend(0.0, step, float8ops::blend ( target - cur_value , cur_value - target, prev_value > target) > float8 (0.0f) );
	cur_value += step;
	

	return cur_value;
}

}
}
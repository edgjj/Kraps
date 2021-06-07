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

LinearSmoother::LinearSmoother(double& val) : raw_value (val)
{
	new_value = raw_value;
	time_cst = 0.2;
}

void LinearSmoother::set_sample_rate(double _sample_rate)
{
	sample_rate = _sample_rate; 
	sr_cst = 1.0 / sample_rate;
}

LinearSmoother::~LinearSmoother()
{

}

double LinearSmoother::get_smoothed_value()
{
	if (sample_rate == 0.0)
		return 0.0;


	if (abs (new_value - raw_value) > 0.0001f)
	{
		prev_value = new_value;
		new_value = raw_value;
		frac = 0.0;
	}

	if (frac < 1.0)
		frac += sr_cst / time_cst;


	return new_value * frac + prev_value * (1 - frac);
}

}
}
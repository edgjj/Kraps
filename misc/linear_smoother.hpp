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

#ifndef KRAPSSMOOTHER_H
#define KRAPSSMOOTHER_H
#include <cstdint>
#include "../simd/avir_float8_avx.h"
namespace kraps
{
namespace misc
{

class LinearSmoother
{
public:
	LinearSmoother();
	void set_target(const float8& v);
	void set_time_cst(const float8& v);

	float8 get_next_value();
	//void set_smoothing_time(double time) { smoothing_time = time; }
	void set_sample_rate(double _sample_rate);

	~LinearSmoother();
private:
	uint8_t counter = 0;

	double sample_rate = 0.0;
	float8 time_cst = 1.0;
	double sr_cst = 0.0;

	float8 cur_value = 0.0;

	float8 step = 0.0;
	float8 target = 0.0;
	float8 prev_value = 0.0;
};

}
}
#endif

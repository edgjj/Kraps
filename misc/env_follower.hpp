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


#include "../simd/avir_float8_avx.h"

#ifndef KRAPS_ENVFOLLOWER_H
#define KRAPS_ENVFOLLOWER_H
namespace kraps
{
namespace misc
{

class EnvelopeFollower
{
public:
	EnvelopeFollower() { ; }
	~EnvelopeFollower() { ; }

	void setup(const float& _sample_rate, const float8& a, const float8& r);

	float8 process(const float8& x);
private:
	float8 z1 = 0.f;

	float8 coeffs[2];
};

}
}



#endif
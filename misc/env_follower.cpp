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


#include "env_follower.hpp"

namespace kraps
{
namespace misc
{

void EnvelopeFollower::setup (const float& sample_rate, const float8& a, const float8& r)
{
	float8 cst1 = 0.01, cst2 = 1.0;
    coeffs[0] = pow256_ps(cst1, cst2 / (a * float8 (sample_rate) ) );
    coeffs[1] = pow256_ps(cst1, cst2 / (r * float8 (sample_rate) ) );
}


float8 EnvelopeFollower::process(const float8& x)
{
    float8 f = sfabs(x);
    if (f > z1)
        z1 = coeffs[0] * (z1 - f) + f;
    else
        z1 = coeffs[1] * (z1 - f) + f;

    return z1;
}

}
}

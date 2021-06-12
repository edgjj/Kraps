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

#include "../processor/processor.hpp"

namespace kraps
{
namespace misc
{
class Decomposer : public Processor
{
public:
	Decomposer() : Processor (p_decomposer, 2, 16) { ; }
	~Decomposer() { ; }
	void process_callback() override
	{
		float data[8];
		for (int i = 0; i < inputs.size(); i++)
		{
			inputs[i]->src->val.storeu(data);
			for (int o = 0; o < 8; o++)
				*outputs[o + i*8] = float8(data[o]);
		}
	}

private:
	
};
}
}
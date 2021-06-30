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
 
#ifndef KRAPS_TUBEDIST_H
#define KRAPS_TUBEDIST_H
#include "../processor/processor.hpp"

namespace kraps 
{
namespace dafx
{

class TubeDist : public Processor
{
public:
	enum kTubeInputs
	{
		kTubeDistAudioIn
	};
	enum kTubeOutputs
	{
		kTubeDistAudioOut
	};

	TubeDist();
	~TubeDist();

	
	void process_params() override;
	void process_callback() override;
private:
	float8 tC = 0, bC = 0, pC = 0;
	float8 pre_gain = 0, gain = 0, out_gain = 0;
};
}
}


#endif 

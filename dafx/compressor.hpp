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

#ifndef KRAPS_COMPRESSOR_H
#define KRAPS_COMPRESSOR_H

#include "../processor/processor.hpp"
#include "../misc/env_follower.hpp"

namespace kraps
{
namespace dafx
{


class Compressor : public Processor
{
public:
	enum kCompressorInputs
	{
		kDAFXAudioIn
	};
	enum kCompressorOutputs
	{
		kDAFXAudioOut
	};


	Compressor();
	~Compressor();

	void process_params() override;
	void process_callback() override;
	void recalculate_sr() override;

private:
	float8 threshold, ratio, knee, out_gain;

	misc::EnvelopeFollower follower;

};

}
}


#endif


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

#ifndef KRAPS_PULVERIZER_H
#define KRAPS_PULVERIZER_H
#include "../processor/processor.hpp"

namespace kraps
{
namespace filters
{

struct BiquadCoeffs
{
	float8 a0, a1, a2;
	float8 b0, b1, b2;
};

struct BiquadDF1UnitD
{
	float8 z1[2];
	float8 z2[2];
};

enum kPulverizerInputs
{
	kPulverizerAudioIn,
	kPulverizerFreqIn,
	kPulverizerResIn,
	kPulverizerAudioIn2,
	kPulverizerFreqIn2,
	kPulverizerResIn2,
};

enum kPulverizerOutputs
{
	kPulverizerAudioOut1,
	kPulverizerAudioOut2
};

class Pulverizer : public Processor
{
public:
	Pulverizer();
	~Pulverizer();

	void process_callback() override;
	void process_params() override;
	void recalculate_sr() override;


private:
	void calc_cores();
	float8 ssin(const float8& in);


	float8 process_stage(float8 in, const bool& is_second, const int& num);

	float8 precalc_sr;


	std::array <BiquadDF1UnitD, 100> cores_1 = { BiquadDF1UnitD () };
	std::array <BiquadDF1UnitD, 100> cores_2 = { BiquadDF1UnitD () };

	BiquadCoeffs c1, c2;

	float8 freq1, freq2, q1, q2;

	float amts[2];


};


}
}
#endif
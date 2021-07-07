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


#include "pulverizer.hpp"

namespace kraps
{
namespace filters
{
Pulverizer::Pulverizer() : Processor (p_pulverizer, 6, 2)
{
	pt = kraps::parameter::pt::ParameterTable(
		{ new parameter::Parameter<float>("frequency1", 400, 400, 30, 20000),
		 new parameter::Parameter<float>("res1", 0.5, 0.5, 0.5, 16),
		 new parameter::Parameter<int>("amount1", 4, 4, 0, 100),
		 new parameter::Parameter<float>("frequency2", 400, 400, 30, 20000),
		 new parameter::Parameter<float>("res2", 0.5, 0.5, 0.5, 16),
		 new parameter::Parameter<int>("amount2", 4, 4, 0, 100),
		});


	io_description[0] =
	{
		{kPulverizerAudioIn, "AUDIO1", "1st audio input."},
		{kPulverizerFreqIn, "FREQ1", "Frequency mod. input. (1st)"},
		{kPulverizerResIn, "RES1", "Q mod. input. (1st)"},
		{kPulverizerAudioIn2, "AUDIO1", "2nd audio input."},
		{kPulverizerFreqIn2, "FREQ1", "Frequency mod. input. (2nd)"},
		{kPulverizerResIn2, "RES1", "Q mod. input. (2nd)"},
	};

	io_description[1] =
	{
		{kPulverizerAudioOut1, "AUDIO1", "1st Output for processed signal."},
		{kPulverizerAudioOut2, "AUDIO2", "2nd Output for processed signal."},
	};

}
Pulverizer::~Pulverizer()
{

}

void Pulverizer::recalculate_sr()
{
	precalc_sr = 1.0 / sample_rate;
	calc_cores();
}

float8 Pulverizer::ssin(const float8& v)
{
	return (float8(16) * v * (float8(M_PI) - v))
		/ (float8(5 * M_PI * M_PI) - float8(4) * v * (float8(M_PI) - v));

}

void Pulverizer::calc_cores()
{

	float8 omega_0 = float8 (2 * M_PI) * freq1 * precalc_sr;
	float8 alpha = ssin(omega_0) / ( float8 (2) * q1);

	c1.a0 = float8(1) + alpha;
	c1.b0 = ( float8(1) - alpha ) / c1.a0;
	c1.b1 = ( float8 (-2) * ssin( float8 (M_PI_2) - omega_0) ) / c1.a0;
	c1.b2 = 1;
	c1.a1 = c1.b1;
	c1.a2 = c1.b0;

	omega_0 = float8(2 * M_PI) * freq2 * precalc_sr;
	alpha = ssin(omega_0) / ( float8(2) * q1 );

	c2.a0 = float8 (1) + alpha;
	c2.b0 = ( float8(1) - alpha ) / c2.a0;
	c2.b1 = ( float8(-2) * ssin(float8(M_PI_2) - omega_0) ) / c2.a0; // cos = sin (pi/2 - x)
	c2.b2 = 1;
	c2.a1 = c2.b1;
	c2.a2 = c2.b0;

}

void Pulverizer::process_params()
{
	float8 cst = 20000;

	freq1 = pt.get_raw_value("frequency1");
	freq1 += *inputs[kPulverizerFreqIn] * cst;
	freq1 = float8ops::clamp (freq1, 30, cst);

	freq2 = pt.get_raw_value("frequency2");
	freq2 += *inputs[kPulverizerFreqIn2] * cst;
	freq2 = float8ops::clamp(freq2, 30, cst);

	q1 = pt.get_raw_value("res1") + *inputs[kPulverizerResIn];
	q2 = pt.get_raw_value("res2") + *inputs[kPulverizerResIn2];

	amts[0] = pt.get_raw_value("amount1");
	amts[1] = pt.get_raw_value("amount2");

	calc_cores();
}

float8 Pulverizer::process_stage(float8 in, const bool& is_second = true, const int& num_stage = 0)
{
	const BiquadCoeffs& _c = is_second == true ? c2 : c1;
	std::array <BiquadDF1UnitD, 100>& _cores = is_second == true ? cores_2 : cores_1;


	float8 ret = in * _c.b0 + _cores[num_stage].z1[0] * _c.b1 + _cores[num_stage].z2[0] * _c.b2
							- _cores[num_stage].z1[1] * _c.a1 - _cores[num_stage].z2[1] * _c.a2;

	_cores[num_stage].z2[0] = _cores[num_stage].z1[0];
	_cores[num_stage].z2[1] = _cores[num_stage].z1[1];
	_cores[num_stage].z1[0] = in;
	_cores[num_stage].z1[1] = ret;

	return ret;

}

void Pulverizer::process_callback()
{

	float8 out1 = *inputs[kPulverizerAudioIn], out2 = *inputs[kPulverizerAudioIn2];

	for (int j = 0; j < amts[0]; j++)
		out1 = process_stage( out1, false, j);

	for (int j = 0; j < amts[1]; j++)
		out2 = process_stage( out2, true, j);

	*outputs[kPulverizerAudioOut1] = out1;
	*outputs[kPulverizerAudioOut2] = out2;


}




}
}
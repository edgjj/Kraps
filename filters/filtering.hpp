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

#ifndef KRPSFILTER_H
#define KRPSFILTER_H

#include "../processor/processor.hpp"
#include "../misc/linear_smoother.hpp"

namespace kraps
{
namespace filters
{
enum kFilterInputs
{
	kFilterAudioIn,
	kFilterFreqIn,
	kFilterResIn
};

enum kFilterOutputs
{
	kFilterAudioOutLPF,
	kFilterAudioOutHPF,
	kFilterAudioOutBPF,
	kFilterAudioOutAPF,
	kFilterAudioOutNF,
	kFilterAudioOutPF
};
class Filter : public Processor
{
public:
	Filter();
	~Filter();
	void recalculate_sr() override;
	void process_callback() override;
	void process_params() override;


private:
	void calc_filter();
	float8 ftan(const float8& v);


	float8 freq, qfac, param_order;

	float8 gCoeff, RCoeff, KCoeff;		// gain element 

	float8 z1_A, z2_A;		// state variables (z^-1)


};
}
}


#endif

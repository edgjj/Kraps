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
#include "DSPFilters/include/DspFilters/Dsp.h"

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
	kFilterAudioOutBPF
};
class Filter : public Processor
{
public:
	Filter();
	~Filter();
	void recalculate_sr() override;
	void process_callback() override;
private:
	void setup_filtering();
	float8 freq = 0.0;
	std::vector <std::unique_ptr <Dsp::Filter> > filters_bank;
	Dsp::Params f_params;

	

	double** fake_ptr;
};
}
}


#endif

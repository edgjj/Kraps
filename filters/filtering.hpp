#ifndef KRPSFILTER_H
#define KRPSFILTER_H

#include "../processor/processor.hpp"
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
	kFilterAudioOutBPF,
	kFilterAudioOutAPF
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
	std::vector <std::unique_ptr <Dsp::Filter> > filters_bank;
	Dsp::Params f_params;
	double** fake_ptr;
};
}
}


#endif

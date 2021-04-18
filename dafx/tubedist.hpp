#ifndef KRAPSDAFX_H
#define KRAPSDAFX_H
#include "../processor/processor.hpp"

namespace kraps 
{
namespace dafx
{
enum kTubeInputs 
{
	kDAFXAudioIn
};
enum kTubeOutputs
{
	kDAFXAudioOut
};
class TubeDist : public Processor
{
public:
	TubeDist();
	~TubeDist();
	void process_params() override;
	void process_callback() override;
private:
	double tC = 0, bC = 0, pC = 0;
	double pre_gain = 0, gain = 0, out_gain = 0;
};
}
}


#endif 

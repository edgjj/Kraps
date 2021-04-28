#include "delay.hpp"

namespace kraps
{
namespace dafx
{
Delay::Delay() : Processor (p_delay, 1, 1)
{
	params = { 0.02, 0.7, 0.5 };
	params_constrainments = { {0, 5}, {0, 1}, {0, 1} };
	dly_line = std::make_unique<DelayLine <double>>(5 * 44100, 44100);

	io_description[0] =
	{
		{kDelayAudioIn, "AUDIO", "Just audio input."}
	};

	io_description[1] =
	{
		{kDelayAudioIn, "AUDIO", "Output for processed signal."}
	};
}

Delay::~Delay()
{
	;
}
void Delay::recalculate_sr()
{
	dly_line = std::make_unique<DelayLine <double>>(5 * sample_rate, sample_rate);
}
void Delay::process_callback()
{
	double fbsmp = dly_line->get_interp(params[0]);
	dly_line->push(*inputs[kDelayAudioIn] + fbsmp * params[1]);
	*outputs[kDelayAudioOut] = *inputs[kDelayAudioIn] + fbsmp * params[2];
}
}
}
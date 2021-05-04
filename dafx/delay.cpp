#include "delay.hpp"

namespace kraps
{
namespace dafx
{
Delay::Delay() : Processor (p_delay, 2, 1)
{
	params = { 0.02, 0.7, 0.5 };
	params_constrainments = { {0.001, 0.5}, {0, 1}, {0, 1} };
	dly_line = std::make_unique<DelayLine <double>>(2 * 44100, 44100);
	smoother = std::make_unique<misc::LeakySmoother>(smoothed_time);


	io_description[0] =
	{
		{kDelayAudioIn, "AUDIO", "Just audio input."},
		{kDelayTimeIn, "TIME", "Time modulation input."}
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
	dly_line = std::make_unique<DelayLine <double>>(2 * sample_rate, sample_rate);
	smoother->set_sample_rate(sample_rate);
}

void Delay::process_params()
{
	param_time = params[0];
}
void Delay::process_callback()
{
	smoothed_time = fmax (fmin(*inputs[kDelayTimeIn] + param_time, 0.5), 0.001);

	double fbsmp = dly_line->get_interp(smoother->get_smoothed_value());
	fbsmp = isnan(fbsmp) ? 0.0 : fbsmp;
	dly_line->push(*inputs[kDelayAudioIn] + fbsmp * params[1]);
	*outputs[kDelayAudioOut] = *inputs[kDelayAudioIn] + fbsmp * params[2];
}
}
}
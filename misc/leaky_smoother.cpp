#include "leaky_smoother.hpp"
#include <cmath>
namespace kraps
{
namespace misc
{

LeakySmoother::LeakySmoother(double& val) : raw_value (val)
{
	prev_value = raw_value;
	time_cst = 0.01;
}

void LeakySmoother::set_sample_rate(double _sample_rate)
{
	sample_rate = _sample_rate; 
	lmbda = exp(-1 / (time_cst * sample_rate));
}
LeakySmoother::~LeakySmoother()
{

}

double LeakySmoother::get_smoothed_value()
{
	if (sample_rate == 0.0)
		return 0.0;

	double out = (1 - lmbda) * raw_value + lmbda * prev_value;
	prev_value = out;
	return out;

}

}
}
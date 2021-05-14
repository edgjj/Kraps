#include "linear_smoother.hpp"
#include <cmath>
namespace kraps
{
namespace misc
{

LinearSmoother::LinearSmoother(double& val) : raw_value (val)
{
	prev_value = raw_value;
	time_cst = 0.2;
}

void LinearSmoother::set_sample_rate(double _sample_rate)
{
	sample_rate = _sample_rate; 
	sr_cst = 1.0 / sample_rate;
}
LinearSmoother::~LinearSmoother()
{

}

double LinearSmoother::get_smoothed_value()
{
	if (sample_rate == 0.0)
		return 0.0;


	if (abs (new_value - raw_value) > 0.0001f)
	{
		prev_value = new_value;
		new_value = raw_value;
		frac = 0.0;
	}

	if (frac < 1.0)
		frac += sr_cst / time_cst;


	return prev_value * frac + new_value * (1 - frac);
}

}
}
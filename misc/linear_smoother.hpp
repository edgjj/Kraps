#ifndef KRAPSSMOOTHER_H
#define KRAPSSMOOTHER_H
#include <cstdint>
namespace kraps
{
namespace misc
{

class LinearSmoother
{
public:
	LinearSmoother(double& val);
	double get_smoothed_value();
	//void set_smoothing_time(double time) { smoothing_time = time; }
	void set_sample_rate(double _sample_rate);
	~LinearSmoother();
private:
	double cr = 4096;
	uint8_t counter = 0;

	double sample_rate = 0.0;
	double time_cst = 0.0;
	double sr_cst = 0.0;

	double prev_value = 0.0, new_value = -1.0;

	double frac = 1.0;


	const double& raw_value;
};

}
}
#endif

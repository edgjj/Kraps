#ifndef KRAPSSMOOTHER_H
#define KRAPSSMOOTHER_H
namespace kraps
{
namespace misc
{

class LeakySmoother
{
public:
	LeakySmoother(double& val);
	double get_smoothed_value();
	//void set_smoothing_time(double time) { smoothing_time = time; }
	void set_sample_rate(double _sample_rate);
	~LeakySmoother();
private:
	double sample_rate = 0.0;
	double time_cst = 0.0;
	double lmbda = 0.0;

	double prev_value = 0.0;

	const double& raw_value;
};

}
}
#endif

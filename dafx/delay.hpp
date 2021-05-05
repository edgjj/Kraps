#ifndef KRPSDELAY_H
#define KRPSDELAY_H
#include "../processor/processor.hpp"
#include "../misc/leaky_smoother.hpp"
namespace kraps
{
namespace dafx
{

enum kDelayInputs
{
    kDelayAudioIn,
    kDelayTimeIn
};

enum kDelayOutputs
{
    kDelayAudioOut
};

template<typename T>
class DelayLine {
public:
    DelayLine(int numSamples, double smp_rate) {
        sample_cnt = numSamples;
        sample_rate = smp_rate;
        raw_buf.resize(sample_cnt);
    };
    ~DelayLine() {
    }
    void push(T smp) {
        raw_buf[w_pos] = smp;
        w_pos = w_pos == 0 ? sample_cnt - 1 : w_pos - 1;
    }
    T get(int time) {
        return raw_buf[(w_pos + time) % sample_cnt];
    }
    T get_interp(float time) {
        double src_time = time * sample_rate;
        double trunc_t = (int)src_time;
        double frac = src_time - (int)src_time;

        double ret = get(trunc_t + 1) * pow(frac, 3) / 6
            + get(trunc_t) * (pow(1 + frac, 3) - 4 * pow(frac, 3)) / 6
            + get(trunc_t - 1) * (pow(2 - frac, 3) - 4 * pow(1 - frac, 3)) / 6
            + get(trunc_t - 2) * pow(1 - frac, 3) / 6;

        return ret;

    }

private:
    std::vector<T> raw_buf;
    int w_pos = 0;
    int sample_cnt = 0;
    double sample_rate = 0.0;
};

class Delay : public Processor
{
public:
	Delay();
	~Delay();
    void process_callback() override; 
    void recalculate_sr() override;
    void process_params() override;
private:
    std::unique_ptr<misc::LeakySmoother> smoother;

    std::unique_ptr<DelayLine <double>> dly_line;
    double param_time = 0.0;
    double smoothed_time = 0.0;
};

}
}

#endif

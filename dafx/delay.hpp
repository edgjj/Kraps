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

#ifndef KRPSDELAY_H
#define KRPSDELAY_H
#include "../processor/processor.hpp"
#include "../misc/linear_smoother.hpp"
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
        int norm = w_pos + time;
        norm = norm < 0 ? norm += sample_cnt : norm;
        return raw_buf[norm % sample_cnt];
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
    std::unique_ptr<misc::LinearSmoother> smoother;

    std::unique_ptr<DelayLine <double>> dly_line;
    double param_time = 0.0;
    double smoothed_time = 0.0;
};

}
}

#endif

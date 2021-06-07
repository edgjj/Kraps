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

template<size_t MaxTime>
class AVXDelayLine {
public:
    AVXDelayLine()
    {    
    };
    ~AVXDelayLine()
    {
    }

    void set_sample_rate(double sr)
    {
        sample_rate = sr;
        sample_cnt = max_time * sample_rate; 
        w_pos = 0;
        raw_buf.clear();
        raw_buf.resize(sample_cnt + 1);
             
    }

    void push(const float8& smp) 
    {
        assert(sample_rate != 0);

        raw_buf[w_pos] = smp;
        w_pos = w_pos == 0 ? sample_cnt - 1 : w_pos - 1;
    }
    float8 get(float8 time) 
    {    
        int norm = w_pos + time;
        norm = norm < 0 ? norm += sample_cnt : norm; // calc custom sample for each time in packed float

        return raw_buf[norm % sample_cnt];
    }

    float8 get_interp(float8 time) 
    {
        assert(sample_rate != 0);

        float8 time8 = float8(time);

        float8 src_time = time8 * float8(sample_rate);
        float8 trunc_t = roundneg (src_time);
            
        float8 frac = src_time - trunc_t;
        
        float8 ret = get(trunc_t + float8(1)) * frac + get(trunc_t) * (float8(1) - frac);

       /* float8 ret = get(trunc_t + float8(1)) * pow(frac, 3) / float8 (6)
            + get(trunc_t) * (pow(float8 (1) + frac, 3) - 4 * pow(frac, 3)) / float8 (6)
            + get(trunc_t - float8(1)) * (pow(float8(2) - frac, 3) - 4 * pow(float8(1) - frac, 3)) / float8 (6)
            + get(trunc_t - float8(2)) * pow(float8(1) - frac, 3) / float8 (6);*/

        return ret;

    }

private:
    std::vector<float8> raw_buf;
    int w_pos;

    double max_time = MaxTime;

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
    std::unique_ptr <misc::LinearSmoother> smoother;


    AVXDelayLine <5> dly_line;

    double param_time = 0.0;
    double smoothed_time = 0.0;
};

}
}

#endif

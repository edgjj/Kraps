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

#ifndef KRAPS_DELAY_H
#define KRAPS_DELAY_H
#include "../processor/processor.hpp"
#include "../misc/linear_smoother.hpp"
namespace kraps
{
namespace dafx
{


template<size_t MaxTime>
class AVXDelayLine {
public:
    AVXDelayLine()
    {    
    };
    ~AVXDelayLine()
    {
    }

    void set_sample_rate(float sr)
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

        return ret;

    }

private:
    std::vector<float8> raw_buf;
    int w_pos;

    float max_time = MaxTime;

    int sample_cnt = 0;
    float sample_rate = 0.0;
};

class Delay : public Processor
{
public:
    enum kDelayInputs
    {
        kDelayAudioIn,
        kDelayTimeIn
    };

    enum kDelayOutputs
    {
        kDelayAudioOut
    };


	Delay();
	~Delay();
    void process_callback() override; 
    void recalculate_sr() override;
    void process_params() override;
private:
    misc::LinearSmoother smoother;


    AVXDelayLine <5> dly_line;

    float8 time = 0.0, drywet = 0.0, feedback = 0.0;
};

}
}

#endif

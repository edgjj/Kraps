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

#include "sampler.hpp"
#include "../serialize/base64/base64.hpp"

namespace kraps
{
Sampler::Sampler() : Processor (p_sampler, 3, 1)
{
    params = { 261.64, 0.0, 0.0 };
    base_freq = 261.63;
    params_constrainments.push_back(std::pair(1.0, 5000));
    params_constrainments.push_back(std::pair(0.0, 1.0));
    params_constrainments.push_back(std::pair(0.0, 1.0));

    io_description[0] =
    {
        {
            kSamplerFreqIn, "FREQ", "Frequency input."
        },
        {
            kSamplerPhaseIn, "PHASE", "Phase modulation input."
        },
        {
            kSamplerGateIn, "GATE", "Just gate. Resets sampler phase."
        }
    };

    io_description[1] =
    {
        {
            kSamplerAudioOut, "AUDIO", "Provides raw audio."
        }
    };
}
Sampler::~Sampler()
{

}
void Sampler::load_source(float* buf, size_t len, double stream_sample_rate)
{
	set_lock();


	cur_file_len = len > max_len ? max_len : len;
	source.reset(new double[cur_file_len]);

	for (uint32_t i = 0; i < cur_file_len; i++)
		source[i] = buf[i];

    params_constrainments[1].second = cur_file_len;
    file_sample_rate = stream_sample_rate;
    pos = cur_file_len - 1;


    set_unlock();
}


void Sampler::load_source_unserialize(double* buf)
{

    if (cur_file_len == 0)
        return;
  
    source.reset(new double[cur_file_len]);

    std::memcpy(source.get(), buf, cur_file_len * sizeof(double));

    params_constrainments[1].second = cur_file_len;
    pos = cur_file_len - 1;


}

uint32_t Sampler::get_max_len()
{
    return max_len;
}

uint32_t Sampler::get_source_length()
{
    return cur_file_len;
}

double* Sampler::get_source_view() const
{
	return source.get();
}

void Sampler::process_callback()
{
    if (source.get() == nullptr || cur_file_len == 0)
        return;

    upd_freq();

    /*
        basically what we should do:
        if we play file at:
            original sample rate: we have original pitch and length
            2x sample rate: 1 oct higher
            4x sample rate: 2 oct higher
            0.5x sample rate: 1 oct lower
            
            ..etc
    
    */

    float8 cur_pos = pos;
    float8 pos_int = roundneg (cur_pos);
    float8 pos_int_inc = pos_int + float8(1);

    float8 pos_frac = cur_pos - roundneg(cur_pos);

    *outputs[kSamplerAudioOut] = blend(source[pos_int] * (1 - pos_frac) + source[pos_int_inc] * pos_frac, float8(0.f), pos == float8(cur_file_len - 1));

    inc_phase();
}

void Sampler::process_params()
{
    base_freq = params[0];
    is_looping = params[2];    
}
double Sampler::get_position()
{
    return pos;
}
void Sampler::upd_freq()
{
    float8 freq = smin( *inputs[kSamplerFreqIn], float8 (sample_rate / 2) );
    double c3 = 261.63;
    if (freq == float8 (0.f))
        freq = c3;
    phase_inc = (file_sample_rate / sample_rate) * freq / base_freq;
}
void Sampler::inc_phase()
{

    float8 cmp = *inputs[kSamplerGateIn] != gate;

    if (movemask(cmp) != 0)
    {
        float8 cmp_gate = andnot(gate, *inputs[kSamplerGateIn]) == float8(1.0f);
        pos = blend(pos, float8(params[1]), cmp_gate);
        gate = *inputs[kSamplerGateIn];
    }

    pos += *inputs[kSamplerPhaseIn] + phase_inc;
    if (is_looping == false)
        pos = fmax(fmin(pos, cur_file_len - 1), 0.0);
    else
        pos = pos.operator float() >= cur_file_len - 1 ? 0 : pos;

}

nlohmann::json Sampler::get_serialize_obj()
{
    set_lock();

    nlohmann::json o;
    o["source"] = base64_encode((BYTE*)source.get(), sizeof(double) * cur_file_len);
    o["cur_file_len"] = cur_file_len;
    o["sample_rate"] = file_sample_rate;

    set_unlock();
    o.update(Processor::get_serialize_obj());

    return o;
}

void Sampler::set_serialize(nlohmann::json obj)
{
    Processor::set_serialize(obj);
    set_lock();

    if (obj.find("cur_file_len") != obj.end())
        obj["cur_file_len"].get_to(cur_file_len);

    if (obj.find("sample_rate") != obj.end())
        obj["sample_rate"].get_to(file_sample_rate);

    if (obj.find("source") != obj.end())
    {
        auto decoded = base64_decode(obj["source"]);
        if (decoded.size() / sizeof(double) != cur_file_len)
        {
            file_sample_rate = 0.0;
            cur_file_len = 0.0;
        }
        else load_source_unserialize((double*)decoded.data());
        
    }

    set_unlock();
}


}
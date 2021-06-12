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
    pt = kraps::parameter::pt::ParameterTable(
        { new parameter::Parameter<float>("base_freq", 261.63, 261.63, 1, 5000),
         new parameter::Parameter<int>("sample_start", 0, 0, 0, 1.0),
         new parameter::Parameter<bool>("is_looping", false, false, false, true)
        });

    base_freq = 261.63;


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

    //params_constrainments[1].second = cur_file_len;
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

    //params_constrainments[1].second = cur_file_len;
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

float8 Sampler::pack_voices(const float8& pos)
{
    float data[8], pos_data[8];

    pos.store(pos_data);

#pragma loop(hint_parallel(8))
    for (int i = 0; i < 8; i++)
        data[i] = source[pos_data[i]];

    float8 ret = ret.loadu(data);

    return ret;
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

    *outputs[kSamplerAudioOut] = blend(pack_voices(pos_int) * (float8(1) - pos_frac) + pack_voices(pos_int_inc) * pos_frac, float8(0.f), pos == float8(cur_file_len - 1));

    inc_phase();
}

void Sampler::process_params()
{
    base_freq = pt.get_raw_value ("base_freq");
    is_looping = pt.get_raw_value ("is_looping");    
}
float8 Sampler::get_position()
{
    set_lock();
    float8 ret = pos;
    set_unlock();
    return ret;
}
void Sampler::upd_freq()
{
    float8 freqin = *inputs[kSamplerFreqIn];
    float8 freq = smin(freqin, float8 (sample_rate / 2) );

    double c3 = 261.63;
    freq = blend(freq, float8(c3), freq == float8(0.f));

    phase_inc = float8(file_sample_rate / sample_rate) * freq / base_freq;
}
void Sampler::inc_phase()
{

    float8 cmp = *inputs[kSamplerGateIn] != gate;

    if (movemask(cmp) != 0)
    {
        float8 cmp_gate = andnot(gate, *inputs[kSamplerGateIn]) == float8(1.0f);
        pos = blend(pos, float8(pt.get_raw_value("sample_start")), cmp_gate);
        gate = *inputs[kSamplerGateIn];
    }
    float8 phase_in = *inputs[kSamplerPhaseIn];
    pos += phase_in + phase_inc;

    if (is_looping == false)
        pos = clamp(pos, float8(0.0), float8(cur_file_len - 1));
    else
        pos = blend(pos, float8(0), pos >= float8(cur_file_len - 1)); 

}

const nlohmann::json Sampler::get_serialize_obj()
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

void Sampler::set_serialize(const nlohmann::json& obj)
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
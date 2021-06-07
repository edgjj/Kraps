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

#ifndef KRPSSAMPLER_H
#define KRPSSAMPLER_H
#include "../processor/processor.hpp"

namespace kraps
{
enum kSamplerInputs
{
	kSamplerFreqIn,
	kSamplerPhaseIn,
	kSamplerGateIn
};
enum kSamplerOutputs
{
	kSamplerAudioOut
};
class Sampler : public Processor
{
public:
	Sampler();
	~Sampler();
	void load_source(float* buf, size_t len, double stream_sample_rate);
	
	float8 get_position();

	void upd_freq();
	double* get_source_view() const;

	
	uint32_t get_max_len();
	uint32_t get_source_length();


	const nlohmann::json get_serialize_obj() override;
	void set_serialize(const nlohmann::json&) override;
protected:
	void process_callback() override;
	void process_params() override;
	void inc_phase();
	float8 gate = 0.0;
	float8 phase_inc = 0.0;
private:
	void load_source_unserialize(double* buf);
	float8 pack_voices(const float8& pos);

	std::unique_ptr<double[]> source;
	bool is_looping = 0;

	double file_sample_rate = 0.0;
	float8 base_freq = 130.81;
	float8 pos = 0;


	uint32_t cur_file_len = 0;
	uint32_t max_len = 44100 * 10;
};
}

#endif
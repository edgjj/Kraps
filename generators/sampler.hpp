#ifndef KRPSSAMPLER_H
#define KRPSSAMPLER_H
#include "../../processor/processor.hpp"

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
	
	void upd_freq();
	double* get_source_view() const;
	uint32_t get_max_len();
	uint32_t get_source_length();


	nlohmann::json get_serialize_obj() override;
	void set_serialize(nlohmann::json) override;
protected:
	void process_callback() override;
	void process_params() override;
	void inc_phase();
	bool gate = 0.0;
	double phase_inc = 0.0;
private:
	void load_source_unserialize(double* buf);


	std::unique_ptr<double[]> source;
	double file_sample_rate = 0.0;
	double base_freq = 130.81;
	double pos = 0;
	uint32_t cur_file_len = 0;
	uint32_t max_len = 44100 * 10;
};
}

#endif
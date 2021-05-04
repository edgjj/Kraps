#include "filtering.hpp"

namespace kraps
{
namespace filters
{
Filter::Filter() : Processor (p_filter, 3, 4)
{
    
    params = { 10000.0, 0.8, 1.0 };
    params_constrainments.push_back(std::pair <double, double>(20, 20000.0));
    params_constrainments.push_back(std::pair <double, double>(0.10, 16.0));
    params_constrainments.push_back(std::pair <double, double>(1.0, 16.0));


    setup_filtering();

    fake_ptr = new double* [1];
    fake_ptr[0] = new double[1];
    fake_ptr[0][0] = 0.0;

    io_description[0] =
    {
        {kFilterAudioIn, "AUDIO", "Just audio input."},
        {kFilterFreqIn, "FREQ", "Input for frequency CV."},
        {kFilterResIn, "RES", "Input for resonance CV."},
    };

    io_description[1] =
    {
        {kFilterAudioOutLPF, "LPF", "Low-pass filter output."},
        {kFilterAudioOutHPF, "HPF", "High-pass filter output."},
        {kFilterAudioOutBPF, "BPF", "Band-pass filter output."},
        {kFilterAudioOutAPF, "APF", "All-pass filter output."},
    };

}

Filter::~Filter()
{
    delete fake_ptr[0];
    delete fake_ptr;
}

void Filter::setup_filtering()
{
    filters_bank.emplace_back(std::make_unique <Dsp::SmoothedFilterDesign<Dsp::RBJ::Design::LowPass, 1>>(512));
    filters_bank.emplace_back(std::make_unique <Dsp::SmoothedFilterDesign<Dsp::RBJ::Design::HighPass, 1>>(512));
    filters_bank.emplace_back(std::make_unique <Dsp::SmoothedFilterDesign<Dsp::RBJ::Design::BandPass1, 1>>(512));
    filters_bank.emplace_back(std::make_unique <Dsp::SmoothedFilterDesign<Dsp::RBJ::Design::AllPass, 1>>(512));

    for (auto& i : filters_bank)
    {
        f_params = { 44100.0, params[0], params[1] };
        i->setParams(f_params);
    }
        
}

void Filter::recalculate_sr()
{
    
    f_params[0] = sample_rate;
    for (auto& i : filters_bank)
    {
        i->reset();
        i->setParam(Dsp::ParamID::idSampleRate, sample_rate);
    }
       
}

void Filter::process_callback()
{
    if (sample_rate == 0.0)
        return;

    freq = *inputs[kFilterFreqIn] * ((sample_rate - 2000) / 2) + params[0];
    freq = fmax (fmin(freq, (sample_rate - 2000) / 2), 20.0);

    double q = *inputs[kFilterResIn] + params[1];
    f_params[1] = freq;
    f_params[2] = q;

    
    for (int i = 0; i < filters_bank.size(); i++)
    {
        fake_ptr[0][0] = *inputs[kFilterAudioIn];
        filters_bank[i]->setParams(f_params);
        filters_bank[i]->process(1, fake_ptr);

        *outputs[i] = fake_ptr[0][0];
    }

    // do parallel processing of 4 filter types: LP, HP, BP, AP
    // also we need parameter smoothing
}

}
}
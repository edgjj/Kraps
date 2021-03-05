#ifndef APROCESSOR_H
#define APROCESSOR_H

#include <vector>
#include <cstdint>
#include <array>

#include "../misc/misc.hpp"

typedef enum ProcessorTypes 
{
    p_generator = 1,
    p_modulator = 2,
    p_filter = 4,
    p_dafx = 8,
} ProcessorTypes;

class Processor
{
public:
    Processor (uint8_t type, double sr) : 
        type (type), 
        sample_rate (sr), 
        bypass (false), 
        amp (0.0) { ; }

    virtual void process () = 0;
    virtual void process_params () = 0;

    void set_bypassed (double val) { this->bypass = val; }
    void set_amp (double val) { this->amp = val; }

    inline void set_sample (double sample, int chan) { outputs[chan] = sample; }
    double get_sample (int chan) { return outputs[chan]; }

    

    void set_param (int num, double val) { params[num] = val; process_params(); }
    void set_param (std::vector<double>& val) { params = val; process_params(); }

    double get_param (int num) { return params[num]; }

    bool is_bypassed () { return bypass; }
    uint8_t get_type () { return type; }

    ~Processor() { ; }

protected:
    double sample_rate;
    std::vector<double> params;
private:

    bool bypass;
    uint8_t type;
    std::array<double,2> outputs { 0.0 };
    double amp;

};

#endif
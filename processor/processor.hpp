#ifndef APROCESSOR_H
#define APROCESSOR_H

#include <vector>
#include <cstdint>
#include <array>
#include <memory>

#include "../misc/misc.hpp"

class Processor;

struct Output
{
    Output (Processor* proc)
    {
        this->proc = proc;
        val = 0.0;
    }
    float val;
    Processor* proc;

    inline operator double() const
    {
        return this->val;
    }


    inline const Output& operator=(const double& val)
    {
        this->val = val;
        return *this;
    }

    inline const Output& operator+=(const double& val)
    {
        this->val += val;
        return *this;
    }

    inline const Output& operator*(const double& val)
    {
        this->val *= val;
        return *this;
    }
};

struct Input
{
    Input (Processor* proc, Output* src)
    {
        this->proc = proc;
        this->src = src;
    }

    inline operator double() const
    {
        return src->val;
    }

    
    inline operator float() const
    {
        return (float)src->val;
    }

    inline const bool operator== (const bool& val)
    {
        return src->val == val;
    }

    inline double operator+ (const double& val)
    {
        return src->val + val;
    }

    inline const double operator*(const double& val)
    {
        return src->val * val;
    }

    Processor* proc;
    Output* src;
};

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
    Processor (uint8_t type, uint8_t num_inputs, uint8_t num_outputs);

    virtual void process () = 0;
    

    void set_bypassed (double val) { this->bypass = val; if (val == true) for (auto &i : outputs) i->val = 0.0; }
    void set_amp (double val) { this->amp = val; }
    void set_SR (double val) { sample_rate = val; recalculate_sr (); }
    void set_param (int num, double val) { params[num] = val; process_params(); }
    void set_param (std::vector<double>& val) { params = val; process_params(); }


    double get_SR () { return sample_rate; }
    inline void set_sample (double sample, int num) { outputs[num]->val = sample; }
    double get_sample (int num) { return inputs[num]->src->val; }
    double get_param (int num) { return params[num]; }
    Output* get_out (int num) { return outputs[num].get(); }

    void plug (Output*, uint8_t);
    void plug (Processor*, uint8_t, uint8_t);
    void unplug (uint8_t);
    void unplug (Processor*);


    bool is_bypassed () { return bypass; }
    uint8_t get_type () { return type; }

    ~Processor();

protected:

    virtual void process_params () = 0;
    virtual void recalculate_sr () = 0;

    double sample_rate;
    std::vector<double> params;

    std::vector< std::unique_ptr <Input> > inputs;
    std::vector< std::unique_ptr <Output> > outputs;

private:
    bool bypass;
    uint8_t type;   
    std::unique_ptr<Output> null_out; 
    double amp;
};

#endif
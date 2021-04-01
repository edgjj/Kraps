#ifndef APROCESSOR_H
#define APROCESSOR_H

#include <vector>
#include <cstdint>
#include <array>
#include <memory>


#include "processor_types.hpp"
#include "io.hpp"
#include "../misc/misc.hpp"

static uint32_t ctr = 0;

class Processor
{
public:


    Processor (uint8_t type, uint8_t num_inputs, uint8_t num_outputs);
    

    void set_bypassed (double val) { this->bypass = val; if (val == true) for (auto &i : outputs) i->val = 0.0; }
    void set_SR (double val) { sample_rate = val; recalculate_sr (); process_params(); }
    double get_SR() { return sample_rate; }
    

    void process();
    void set_param(int num, double val) { params[num] = val; process_params(); }
    void set_param(std::vector<double>& val) { params = val; process_params(); }
    uint32_t get_ID () { return id; }
  
    Output* get_output(uint8_t id) { return outputs[id].get(); }

    std::tuple < std::vector <std::unique_ptr<Input> >* , 
        std::vector <std::unique_ptr<Output> >*> 
        get_IO();

    /* plug (output ptr, input_index) */
    void plug (Output*, uint8_t);

    /* plug (input ptr, input_index) */
    void plug(Input*, uint8_t);

    /* plug (src_processor ptr, output_index, input_index) */
    void plug (Processor*, uint8_t, uint8_t);

    /* unplug (input_index) */
    void unplug (uint8_t);

    /* unplug (src_processor ptr) */
    void unplug (Processor*);


    bool is_bypassed () { return bypass; }
    uint8_t get_type () { return type; }

    ~Processor();

protected:

    virtual void process_callback () = 0;
    virtual void process_params () = 0;
    virtual void recalculate_sr () = 0;

    double sample_rate;
    std::vector<double> params;

    std::vector< std::unique_ptr <Input> > inputs;
    std::vector< std::unique_ptr <Output> > outputs;


private:
    const uint32_t id = 0;

    bool bypass;
    uint8_t type;   
    std::unique_ptr<Output> null_out; 
};

#endif
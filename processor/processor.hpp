#ifndef APROCESSOR_H
#define APROCESSOR_H

#include <vector>
#include <cstdint>
#include <array>
#include <memory>

#include "processor_types.hpp"
#include "io.hpp"
#include "../misc/misc.hpp"
#include "../serialize/nlohmann/json.hpp"



#define WAIT_LOCK while (is_locked) { ; } // not thread-safe at all and bad idea

namespace kraps {

struct IODescription
{
    uint32_t id;
    std::string name;
    std::string description;
};

class Processor
{
public:
    Processor (uint8_t type, uint8_t num_inputs, uint8_t num_outputs);

    void set_bypassed(bool val) { WAIT_LOCK;  this->bypass = val; }
    void set_SR (double val) { sample_rate = val; recalculate_sr (); process_params(); }
    double get_SR() { return sample_rate; }
    void set_lock() { is_locked = !is_locked;  }

    void process();

    double get_param(int num) { return params[num]; }
    void set_param(int num, double val) { WAIT_LOCK;  params[num] = val; process_params(); }
    void set_param(std::vector<double>& val) { WAIT_LOCK;  params = val; process_params(); }

    size_t get_param_count() { return params.size(); }
    std::pair <double, double> get_param_range(uint32_t id_) { return params_constrainments[id_]; }
    uint32_t get_ID () { return id; }
    void set_ID(uint32_t _id) { id = _id; }


    io::Output* get_output(uint8_t id_) { return outputs[id_].get(); }

    std::tuple < std::vector <std::unique_ptr<io::Input> >* ,
        std::vector <std::unique_ptr<io::Output> >*>
        get_IO();

    /* plug (output ptr, input_index) */
    void plug (io::Output*, uint8_t);

    /* plug (input ptr, input_index) */
    void plug(io::Input*, uint8_t);

    /* plug (src_processor ptr, output_index, input_index) */
    void plug (Processor*, uint8_t, uint8_t);

    /* unplug everything */
    void unplug();

    /* unplug (input_index) */
    void unplug (uint8_t);

    /* unplug (src_processor ptr) */
    void unplug (Processor*);


    bool is_bypassed () { return bypass; }
    uint8_t get_type () { return type; }



    IODescription get_io_description(uint32_t num, bool is_output = false);

    /* serializing things */
    virtual nlohmann::json get_serialize_obj();
    virtual void set_serialize(nlohmann::json obj);


    virtual ~Processor();

protected:

    virtual void process_callback() { ; }
    virtual void process_params () { ; }
    virtual void recalculate_sr () { ; }
    virtual void process_bypass () { ; }

    /* gui friendly stuff */

    std::array <std::vector <IODescription>, 2> io_description = { };


    /* dsp */

    double sample_rate;
    std::vector<double> params;
    std::vector <std::pair <double, double>> params_constrainments;

    std::vector< std::unique_ptr <io::Input> > inputs;
    std::vector< std::unique_ptr <io::Output> > outputs;

    bool is_locked = false;

private:
    uint32_t id = 0;
    bool bypass;
    uint8_t type;   
    std::unique_ptr<io::Output> null_out;

};

}
#endif
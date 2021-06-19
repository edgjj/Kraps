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

#ifndef APROCESSOR_H
#define APROCESSOR_H

#include <vector>
#include <cstdint>
#include <array>
#include <memory>
#include <mutex>

#include "processor_types.hpp"
#include "io.hpp"
#include "../misc/misc.hpp"
#include "../serialize/nlohmann/json.hpp"
#include "parameters/parameter_table.hpp"

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

    void set_bypassed(bool val) { set_lock();  this->bypass = val; set_unlock();  }
    void set_SR (double val) { sample_rate = val; recalculate_sr (); process_params(); }
    double get_SR() { return sample_rate; }
    void set_lock() { proc_mutex.lock(); }
    void set_unlock() { proc_mutex.unlock(); }

    void process();

    parameter::pt::ParameterTable& get_parameter_table()
    {
        return pt;
    }


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
    virtual const nlohmann::json get_serialize_obj();
    virtual void set_serialize(const nlohmann::json& obj);


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
    
    parameter::pt::ParameterTable pt;

    std::vector< std::unique_ptr <io::Input> > inputs;
    std::vector< std::unique_ptr <io::Output> > outputs;


private:

    std::mutex proc_mutex;

    uint16_t cr_counter = 32;


    uint32_t id = 0;
    bool bypass;
    uint8_t type;   
    std::unique_ptr<io::Output> null_out;

};

}
#endif
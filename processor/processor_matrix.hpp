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

#ifndef APROCESSORMTX_H
#define APROCESSORMTX_H

#include "../generators/wt.hpp"
#include "../modulators/lfo.hpp"
#include "../modulators/adsr.hpp"
#include "../modulators/attenuator.hpp"
#include "../modulators/macros.hpp"
#include "../generators/sampler.hpp"
#include "../filters/filtering.hpp"
#include "../dafx/tubedist.hpp"
#include "../dafx/delay.hpp"
#include "../processor/midi/note_manager.hpp"


typedef std::map <uint32_t, std::tuple< std::vector <std::unique_ptr<kraps::io::Input> >*,
    std::vector <std::unique_ptr<kraps::io::Output> >*> > IO_container;

namespace kraps{

class OutputProcessor : public Processor
{
public:
    OutputProcessor() : Processor(p_output, 1, 0)
    {
        io_description[0] =
        {
            { 0, "AUDIO", "Outputs audio to host."}
        };
    }
    double get_sample() 
    { 
        if (!is_bypassed()) return *inputs[0];
        else return 0.0;
    }
    ~OutputProcessor() { ; }
};


class ProcessorMatrix
{
public:
    ProcessorMatrix ();

    uint32_t add_processor (uint8_t type, uint32_t _id = 0);
    bool remove_processor (uint32_t id);
    void clear();
    Processor* get_processor (uint32_t id);



    void set_SR(double sample_rate);
    void set_lock();
    void set_unlock();


    bool plug(uint32_t src, uint32_t dest, uint16_t src_out, uint16_t dest_in);
    bool unplug(uint32_t dest, uint16_t dest_in);


    NoteManager* get_note_mgr();
    double process ();

    IO_container* get_IO() { return &processors_io; }


    nlohmann::json serialize();
    int deserialize(nlohmann::json o);

    ~ProcessorMatrix ();
private:
    uint32_t proc_ctr = 0;
    IO_container processors_io;

    std::array <uint32_t, 2> immutables;

    std::vector < std::unique_ptr <Processor> > processors;

    double sample_rate = 0;
    std::mutex proc_mutex;
    

};

}
#endif
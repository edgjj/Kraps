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

#include "processor_matrix.hpp"
#include "midi/note_manager.hpp"

namespace kraps {


ProcessorMatrix::ProcessorMatrix()
{
    processors.emplace_back(std::make_unique <OutputProcessor>());
    processors.back()->set_ID(0);
    immutables[0] = 0;
    processors_io[immutables[0]] = processors.back()->get_IO();

    processors.emplace_back(std::make_unique <NoteManager>());
    processors.back()->set_ID(1);
    immutables[1] = 1;
    processors_io[immutables[1]] = processors.back()->get_IO();

    proc_ctr = 2;
}
ProcessorMatrix::~ProcessorMatrix()
{
     // needs independent id counter for each instance
}

uint32_t ProcessorMatrix::add_processor(uint8_t type, uint32_t _id)
{
   
    std::lock_guard<std::mutex> lock(proc_mutex);

    switch (type)
    {
    case p_wt:
        processors.emplace_back(std::make_unique <Wavetable>(2048));
        ((Wavetable*)processors.back().get())->fill_table_from_fcn([](double phase) -> double {
            return sin(phase);
            });
        break;
    case p_lfo:
        processors.emplace_back(std::make_unique <LFO>());
        break;
    case p_adsr:
        processors.emplace_back(std::make_unique <ADSR>());
        break;
    case p_filter:
        processors.emplace_back(std::make_unique <filters::Filter>());
        break;
    case p_tube:
        processors.emplace_back(std::make_unique <dafx::TubeDist>());
        break;
    case p_atten:
        processors.emplace_back(std::make_unique <Attenuator>());
        break;
    case p_summer:
        processors.emplace_back(std::make_unique <Summer>());
        break;
    case p_macro:
        processors.emplace_back(std::make_unique <Macro>());
        break;
    case p_delay:
        processors.emplace_back(std::make_unique <dafx::Delay>());
        break;
    case p_sampler:
        processors.emplace_back(std::make_unique <Sampler>());
        break;
    case p_decomposer:
        processors.emplace_back(std::make_unique<misc::Decomposer>());
        break;
    default:
        return -1;
    }

    processors.back()->set_SR(sample_rate);
    uint32_t id = ++proc_ctr;
    if (_id != 0)
        id = _id;

    processors.back()->set_ID(id);
    processors_io[id] = processors.back()->get_IO();

    
    return id;
}

NoteManager* ProcessorMatrix::get_note_mgr()
{
    return (NoteManager*)processors[1].get();
}

bool ProcessorMatrix::remove_processor(uint32_t id)
{
    std::lock_guard<std::mutex> lock(proc_mutex);

    if (std::find(immutables.begin(), immutables.end(), id) != immutables.end())
        return false;    

    for (auto& i : processors_io)
    {
        auto inputs = std::get<0>(i.second);
        for (auto& j : *inputs)
        {
            if (j->src->id == -1)
                continue;

            if (j->src->proc->get_ID() == id)
                j->proc->unplug(j->id);
        }
    }

    processors.erase(std::remove_if(processors.begin(), processors.end(),
        [&](std::unique_ptr<Processor>& n)
        {
            return n->get_ID() == id;
        }
    ), processors.end());

    processors_io.erase(id);


    return true;
}

void ProcessorMatrix::clear()
{
    std::lock_guard<std::mutex> lock(proc_mutex);

    processors[0]->unplug();
    processors[1]->unplug();

    processors.erase(std::remove_if(processors.begin(), processors.end(),
            [&](std::unique_ptr<Processor>& n)
            {
                return std::find(immutables.begin(), immutables.end(), n->get_ID()) != immutables.end() == false;
            }
    ), processors.end());

    for (auto it = processors_io.begin(); it != processors_io.end(); )
    {
        if (std::find(immutables.begin(), immutables.end(), it->first) != immutables.end() == false)
            it = processors_io.erase(it);
        else 
            ++it;
    }
    

}

Processor* ProcessorMatrix::get_processor(uint32_t id)
{
    auto result = std::find_if(processors.begin(), processors.end(),
        [&](std::unique_ptr<Processor>& n)
        {
            return n->get_ID() == id;
        });

    if (result == processors.end())
        return nullptr;

    return result->get();
}


bool ProcessorMatrix::plug(uint32_t src, uint32_t dest, uint16_t src_out, uint16_t dest_in)
{
    std::lock_guard<std::mutex> lock(proc_mutex);

    if (processors_io.find(src) == processors_io.end() || processors_io.find(dest) == processors_io.end())
        return false;
        

    auto src_vec = std::get <1>(processors_io[src]);
    auto dest_vec = std::get <0>(processors_io[dest]);

    dest_vec->at(dest_in)->src = src_vec->at(src_out).get();


    return true;

}


bool ProcessorMatrix::unplug(uint32_t dest, uint16_t dest_in)
{
    std::lock_guard<std::mutex> lock(proc_mutex);

    if (processors_io.find(dest) == processors_io.end())
        return false;

    auto result = std::find_if(processors.begin(), processors.end(),
        [&](std::unique_ptr<Processor>& n)
        {
            return n->get_ID() == dest;
        });

    result->get()->unplug(dest_in);

}


void ProcessorMatrix::set_SR(double sample_rate)
{
    this->sample_rate = sample_rate;
    for (auto& i : processors)
    {
        i->set_SR(sample_rate);
    }
}

void ProcessorMatrix::set_lock()
{
    proc_mutex.lock();

    for (auto& i : processors)
        i->set_lock();
}

void ProcessorMatrix::set_unlock()
{
    proc_mutex.unlock();

    for (auto& i : processors)
        i->set_unlock();
}


float8 ProcessorMatrix::process()
{
    for (auto& i : processors)
        i->process();


    return ((OutputProcessor*)processors[0].get())->get_sample();
}

const nlohmann::json ProcessorMatrix::serialize()
{
    std::lock_guard<std::mutex> lock(proc_mutex);
    
    using js = nlohmann::json;
    js o;
    for (auto& i : processors)
    {
        o["processors"].push_back(i->get_serialize_obj());
    }


    for (auto& i : processors_io)
    {
        auto id = i.first;
        auto inp = std::get<0>(i.second);

        if (inp->size() == 0)
            continue;

        js r_mtx;
        r_mtx["proc_id"] = id;
        for (auto& i : *inp)
        {
            js input;
            input["id"] = i->id;
            input["src_id"] = i->src->id;
            if (i->src->id == -1)
                input["src_proc_id"] = -1;
            else
                input["src_proc_id"] = i->src->proc->get_ID();

            r_mtx["inputs"].push_back(input);
        }
        o["r_matrix"].push_back(r_mtx);
    }

    return o;
}

int ProcessorMatrix::deserialize(const nlohmann::json& o)
{
    clear();

    using js = nlohmann::json;


    if (o.find("processors") == o.end())
        return -1;

    if (o.find("r_matrix") == o.end())
        return -1;

    for (auto& i : o["processors"])
    {
        uint8_t p_type = i["type"];
        uint32_t p_id = i["id"];

        if (p_type == p_output || p_type == p_notemgr)
            processors[p_id]->set_serialize(i);
        else
        {
            add_processor(p_type, p_id);
            processors.back()->set_serialize(i);
        }
    }

    for (auto& i : o["r_matrix"])
    {
        for (auto& j : i["inputs"])
        {
            if (j["src_id"] == -1)
                continue;
            plug(j["src_proc_id"], i["proc_id"], j["src_id"], j["id"]);
        } 
    }

    proc_ctr = processors.back()->get_ID() + 1;

    return 1;
}


}
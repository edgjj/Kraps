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

#include "processor.hpp"

namespace kraps{

Processor::Processor (uint8_t type, uint8_t num_inputs, uint8_t num_outputs) : 
    type (type), 
    sample_rate (0.0), 
    bypass (false), 
    id (0)
{

    null_out = std::make_unique<io::Output> (nullptr, -1);
    for (int i = 0; i < num_inputs; i++)
    {
        inputs.emplace_back (std::make_unique<io::Input> (this, null_out.get(), i));
    }
    for (int i = 0; i < num_outputs; i++)
    {
        outputs.emplace_back (std::make_unique<io::Output> (this, i));
    }

}

Processor::~Processor ()
{
    
}

void Processor::process ()
{
    /* for future doings */
    if (is_bypassed() == false)
    {
        process_callback();
    }
    else
    {
        for (auto& i : outputs) i->val = 0.0;
        process_bypass();
    }
        
}

std::tuple < std::vector <std::unique_ptr<io::Input> >*, 
    std::vector <std::unique_ptr<io::Output> >*>
    Processor::get_IO()
{
    return std::make_tuple(&inputs, &outputs);
}

void Processor::plug (io::Output* out, uint8_t index)
{
    inputs[index]->src = out;
}

void Processor::plug(io::Input* in, uint8_t index)
{
    inputs[index]->src = in->src;
}

void Processor::plug (Processor* proc, uint8_t o_idx, uint8_t i_idx)
{
    inputs[i_idx]->src = proc->outputs[o_idx].get();
}

void Processor::unplug()
{
    for (auto& i : inputs)
        i->src = null_out.get();
}

void Processor::unplug (uint8_t index)
{
    inputs[index]->src = null_out.get();
}

void Processor::unplug (Processor* proc)
{
    for (uint8_t i = 0; i < inputs.size(); i++)
    {
        if (inputs[i]->proc == proc)
        {
            inputs[i]->src = null_out.get();
            inputs[i]->proc = nullptr;
            return;
        }
    }
}

IODescription Processor::get_io_description(uint32_t num, bool is_output)
{

    if (io_description[is_output].size() <= num)
    {
        std::string out = is_output == true ? "OUT " : "IN ";
        out += std::to_string(num);
        return { num, out, "Empty. " };
    }

    IODescription ret;
    for (auto& i : io_description[is_output])
    {
        if (i.id == num)
        {
            ret = i;
            break;
        }
    }

    return ret;
}

nlohmann::json Processor::get_serialize_obj()
{
    nlohmann::json o;
    o["id"] = id;
    o["params"] = params; 
    o["type"] = type;
    o["bypass"] = bypass;
    return o;
}

void Processor::set_serialize(nlohmann::json obj)
{
    WAIT_LOCK;
    if (obj.find("params") != obj.end())
    {
        std::vector<double> param_bridge;
        obj["params"].get_to(param_bridge);
        for (int p = 0; p < param_bridge.size(); p++)
            params[p] = param_bridge[p];
    }
        
    process_params();
    if (obj.find("bypass") != obj.end())
        obj["bypass"].get_to(bypass);

    if (obj.find("id") != obj.end())
        obj["id"].get_to(id);
}
}
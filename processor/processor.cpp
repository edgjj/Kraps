#include "processor.hpp"

namespace kraps{

Processor::Processor (uint8_t type, uint8_t num_inputs, uint8_t num_outputs) : 
    type (type), 
    sample_rate (0.0), 
    bypass (false), 
    id (ctr++)
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
    ctr--;
}

void Processor::process ()
{
    /* for future doings */
    if (!is_bypassed())
    {
        process_callback();
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
    if (obj.find("params") != obj.end())
        obj["params"].get_to(params);
    if (obj.find("bypass") != obj.end())
        obj["bypass"].get_to(bypass);
}
}
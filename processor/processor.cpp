#include "processor.hpp"

Processor::Processor (uint8_t type, uint8_t num_inputs, uint8_t num_outputs) : 
    type (type), 
    sample_rate (0.0), 
    bypass (false), 
    id (ctr++)
{
    null_out = std::make_unique<Output> (nullptr, -1);
    for (int i = 0; i < num_inputs; i++)
    {
        inputs.emplace_back (std::make_unique<Input> (this, null_out.get(), i));
    }
    for (int i = 0; i < num_outputs; i++)
    {
        outputs.emplace_back (std::make_unique<Output> (this, i));
    }
}

Processor::~Processor ()
{

}

void Processor::process ()
{
    /* for future doings */
    if (!is_bypassed())
    {
        process_callback();
    }
        
}

std::tuple < std::vector <std::unique_ptr<Input> >*, 
    std::vector <std::unique_ptr<Output> >*> 
    Processor::get_IO()
{
    return std::make_tuple(&inputs, &outputs);
}

void Processor::plug (Output* out, uint8_t index)
{
    inputs[index]->src = out;
}

void Processor::plug(Input* in, uint8_t index)
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

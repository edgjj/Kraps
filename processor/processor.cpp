#include "processor.hpp"

Processor::Processor (uint8_t type, uint8_t num_inputs, uint8_t num_outputs) : 
    type (type), 
    sample_rate (0.0), 
    bypass (false), 
    amp (0.0),
    null_out()
{
    for (int i = 0; i < num_inputs; i++)
    {
        inputs.emplace_back (std::make_unique<Input> (this, null_out.get()));
        plug (null_out.get(), i);
    }
    for (int i = 0; i < num_outputs; i++)
    {
        outputs.emplace_back (std::make_unique<Output> (this));
    }
}

Processor::~Processor ()
{

}

void Processor::plug (Output* out, uint8_t index)
{
    inputs[index]->src = out;
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
    for (auto &i : inputs)
    {
        if (i->proc == proc)
        {
            i->src = null_out.get();
            i->proc = nullptr;
            return;
        }
    }
}

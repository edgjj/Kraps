#include "processor.hpp"

Processor::Processor (uint8_t type, uint8_t num_inputs, uint8_t num_outputs) : 
    type (type), 
    sample_rate (0.0), 
    bypass (false), 
    amp (0.0),
    id (ctr++)
{
    null_out = std::make_unique<Output> (nullptr);
    for (int i = 0; i < num_inputs; i++)
    {
        inputs.emplace_back (std::make_unique<Input> (this, null_out.get()));
    }
    for (int i = 0; i < num_outputs; i++)
    {
        outputs.emplace_back (std::make_unique<Output> (this));
    }
}

Processor::~Processor ()
{

}

void Processor::process ()
{
    /* for future doings */
    process_callback ();
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
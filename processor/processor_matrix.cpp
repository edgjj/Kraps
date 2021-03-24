#include "processor_matrix.hpp"

#include "midi/note_manager.hpp"
#define MTX_NUMINPUTS 4
#define MTX_NUMOUTPUTS 4

ProcessorMatrix::ProcessorMatrix()
{
    for (int i = 0; i < MTX_NUMINPUTS; i++)
    {
        global_inputs.emplace_back(std::make_unique<Input>());
    }
    for (int i = 0; i < MTX_NUMOUTPUTS; i++)
    {
        global_outputs.emplace_back(std::make_unique<Output>());
    }

}
ProcessorMatrix::~ProcessorMatrix()
{
    ;
}

uint32_t ProcessorMatrix::add_processor (uint8_t type)
{
    switch (type)
    {
        case p_wt:
            processors.emplace_back ( std::make_unique <Wavetable> (2048));
            break;
        case p_lfo:
            processors.emplace_back ( std::make_unique <LFO> ());
            break;
        case p_adsr:
            processors.emplace_back ( std::make_unique <ADSR> ());
            break;
        case p_filter:
            break;
        case p_dafx:
            break;
    }
    uint32_t id = processors.back()->get_ID();
    processors_io[id] = processors.back()->get_IO();

    return id;
}

void ProcessorMatrix::remove_processor (uint32_t id)
{
    std::remove_if(processors.begin(), processors.end(),
        [&](std::unique_ptr<Processor>& n)
        {
            return n->get_ID() == id;
        }
    );
}

void* ProcessorMatrix::get_processor(uint32_t id)
{
    auto result = std::find_if(processors.begin(), processors.end(),
        [&](std::unique_ptr<Processor>& n)
        {
            return n->get_ID() == id;
        });
    return result->get();
}

void ProcessorMatrix::create_route (uint32_t src, uint32_t dest, uint16_t src_out, uint16_t dest_in)
{
    auto src_vec = std::get <1> (processors_io[src]);
    auto dest_vec = std::get <0> (processors_io[dest]);

    dest_vec->at(dest_in)->src = src_vec->at(src_out).get();
}

void ProcessorMatrix::plug_external(Output* out, uint32_t dest_in)
{
    _ASSERT(dest_in < global_inputs.size());

    global_inputs[dest_in]->src = out;
}

void ProcessorMatrix::set_SR(double sample_rate)
{
    for (auto& i : processors)
    {
        i->set_SR(sample_rate);
    }
}

void ProcessorMatrix::process ()
{
    for (auto& i : processors)
        i->process();
}

void* ProcessorMatrix::serialize ()
{
    return nullptr;
}

Input* ProcessorMatrix::get_in()
{
    return nullptr;
}
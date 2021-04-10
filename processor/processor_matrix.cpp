#include "processor_matrix.hpp"

#include "midi/note_manager.hpp"


ProcessorMatrix::ProcessorMatrix()
{
    processors.emplace_back(std::make_unique <OutputProcessor>());
    immutables[0] = processors.back()->get_ID();
    processors_io[immutables[0]] = processors.back()->get_IO();

    processors.emplace_back(std::make_unique <NoteManager>());
    immutables[1] = processors.back()->get_ID();
    processors_io[immutables[1]] = processors.back()->get_IO();
}
ProcessorMatrix::~ProcessorMatrix()
{
    ;
}

uint32_t ProcessorMatrix::add_processor (uint8_t type)
{
    WAIT_LOCK
    switch (type)
    {
        case p_wt:
            processors.emplace_back ( std::make_unique <Wavetable> (2048));
            //processors.back()->plug(get_in(kMtxGate), kGenGate);
            //processors.back()->plug(get_in(kMtxFreq), kGenFreqIn);
            ((Wavetable*)processors.back().get())->fill_table_from_fcn([](double phase) -> double {
                return sin(phase);
                });
            break;
        case p_lfo:
            processors.emplace_back ( std::make_unique <LFO> ());
            //processors.back()->plug(get_in(kMtxGate), kLFOGate);
            break;
        case p_adsr:
            processors.emplace_back ( std::make_unique <ADSR> ());
            //processors.back()->plug(get_in(kMtxGate), kADSRGate);
            break;
        case p_atten:
            processors.emplace_back(std::make_unique <Attenuator>());
            break;
        case p_macro:
            processors.emplace_back(std::make_unique <Macro>());
            break;
        case p_filter:
            break;
        case p_dafx:
            break;
    }
    processors.back()->set_SR (sample_rate);
    uint32_t id = processors.back()->get_ID();
    processors_io[id] = processors.back()->get_IO();

    return id;
}

NoteManager* ProcessorMatrix::get_note_mgr()
{
    return (NoteManager *) processors[1].get();
}
bool ProcessorMatrix::remove_processor (uint32_t id)
{

    WAIT_LOCK

    if (std::find (immutables.begin(), immutables.end(), id) != immutables.end())
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


bool ProcessorMatrix::plug_internal(uint32_t src, uint32_t dest, uint16_t src_out, uint16_t dest_in)
{
    WAIT_LOCK

    if (processors_io.find(src) == processors_io.end() || processors_io.find(dest) == processors_io.end())
        return false;

    auto src_vec = std::get <1> (processors_io[src]);
    auto dest_vec = std::get <0> (processors_io[dest]);

    dest_vec->at(dest_in)->src = src_vec->at(src_out).get();
    return true;
}


void ProcessorMatrix::plug_external(Output* out, uint32_t dest_in)
{
    WAIT_LOCK
    _ASSERT(dest_in < global_inputs.size());
    global_inputs[dest_in]->src = out;
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
    is_locked = !is_locked;
    for (auto& i : processors)
        i->set_lock();
}

double ProcessorMatrix::process ()
{
    for (auto& i : processors)
        i->process();


    // *global_outputs[kMtxAudioOut] = *std::get<1>(processors_io[output_node])->at(0);

    return ((OutputProcessor*)processors[0].get())->get_sample();
}

void* ProcessorMatrix::serialize ()
{
    return nullptr;
}

Input* ProcessorMatrix::get_in(uint16_t num)
{
    return global_inputs[num].get();
}
#include "processor_matrix.hpp"

#include "../generators/wt.hpp"
#include "../modulators/lfo.hpp"
#include "../modulators/adsr.hpp"


ProcessorMatrix::ProcessorMatrix()
{
    ;
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
            processors.emplace_back ( std::make_unique <Wavetable> ());
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
    return processors.back()->get_ID();
}

void ProcessorMatrix::remove_processor (uint32_t id)
{
    for (int i = 0; i < processors.size(); i++)
    {  
        if (processors[i]->get_ID())
        {
            processors.erase (processors.begin() + i);
            return;
        }
    }
}

void ProcessorMatrix::create_route (uint32_t src, uint32_t dest, uint16_t src_out, uint16_t dest_in)
{

}

void ProcessorMatrix::process ()
{

}

void* ProcessorMatrix::serialize ()
{

}

Output* ProcessorMatrix::get_out ()
{

}
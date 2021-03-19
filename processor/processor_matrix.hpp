#ifndef APROCESSORMTX_H
#define APROCESSORMTX
#include "processor.hpp"

enum kMtxInputs
{
    kMtxGate,
    kMtxNote,
    kMtxPitchWheel
};

enum kMtxOutputs
{
    kMtxAudioOut
};
// m = 12*log2(fm/440 Hz) + 69 and fm =  2(m−69)/12(440 Hz).
class ProcessorMatrix
{
public:
    ProcessorMatrix ();

    uint32_t add_processor (uint8_t type);
    void remove_processor (uint32_t id);

    void create_route (uint32_t src, uint32_t dest, uint16_t src_out, uint16_t dest_in);

    void process ();

    void* serialize ();
    Output* get_out ();

    ~ProcessorMatrix ();
private:

    std::vector < Output* > global_outputs;
    std::vector < std::unique_ptr <Processor> > processors;

};


#endif
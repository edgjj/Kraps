#ifndef APROCESSORMTX_H
#define APROCESSORMTX_H

#include "../generators/wt.hpp"
#include "../modulators/lfo.hpp"
#include "../modulators/adsr.hpp"
#include "../modulators/attenuator.hpp"
#include "../modulators/macros.hpp"
#include "../processor/midi/note_manager.hpp"


typedef std::map <uint32_t, std::tuple< std::vector <std::unique_ptr<kraps::io::Input> >*,
    std::vector <std::unique_ptr<kraps::io::Output> >*> > IO_container;

namespace kraps{


enum kMtxInputs
{
    kMtxGate,
    kMtxFreq,
    kMtxPitchWheel,
    kMtxVeloAmp
};

enum kMtxOutputs
{
    kMtxAudioOut
};

class OutputProcessor : public Processor
{
public:
    OutputProcessor() : Processor(p_output, 1, 0)
    {

    }
    double get_sample() { return *inputs[0]; }
    ~OutputProcessor() { ; }
};


class ProcessorMatrix
{
public:
    ProcessorMatrix ();

    uint32_t add_processor (uint8_t type);
    bool remove_processor (uint32_t id);
    Processor* get_processor (uint32_t id);



    void set_SR(double sample_rate);
    void set_lock();

    bool plug_internal(uint32_t src, uint32_t dest, uint16_t src_out, uint16_t dest_in);
    void plug_external (io::Output* out, uint32_t dest_in);

    NoteManager* get_note_mgr();

    double process ();

    void* serialize ();
    io::Input* get_in (uint16_t num);

    
    IO_container* get_IO() { return &processors_io; }

    ~ProcessorMatrix ();
private:

    IO_container processors_io;

    std::array <uint32_t, 2> immutables;

    std::vector < std::unique_ptr <io::Input>  > global_inputs;
    std::vector < std::unique_ptr <io::Output>  > global_outputs;

    std::vector < std::unique_ptr <Processor> > processors;

    double sample_rate = 0;
    bool is_locked = false;

};

}
#endif
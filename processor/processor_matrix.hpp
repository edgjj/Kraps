#ifndef APROCESSORMTX_H
#define APROCESSORMTX_H


#include "../generators/wt.hpp"
#include "../modulators/lfo.hpp"
#include "../modulators/adsr.hpp"
#include "../modulators/attenuator.hpp"
#include "../modulators/macros.hpp"
#include "../filters/filtering.hpp"
#include "../dafx/tubedist.hpp"
#include "../dafx/delay.hpp"
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

    uint32_t add_processor (uint8_t type, uint32_t _id = 0);
    bool remove_processor (uint32_t id);
    void clear();
    Processor* get_processor (uint32_t id);



    void set_SR(double sample_rate);
    void set_lock();

    bool plug(uint32_t src, uint32_t dest, uint16_t src_out, uint16_t dest_in);
    bool unplug(uint32_t dest, uint16_t dest_in);


    NoteManager* get_note_mgr();
    double process ();

    io::Input* get_in (uint16_t num);

    IO_container* get_IO() { return &processors_io; }


    nlohmann::json serialize();
    int deserialize(nlohmann::json o);

    ~ProcessorMatrix ();
private:
    uint32_t proc_ctr = 0;
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
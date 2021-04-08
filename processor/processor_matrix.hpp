#ifndef APROCESSORMTX_H
#define APROCESSORMTX_H

#include "../generators/wt.hpp"
#include "../modulators/lfo.hpp"
#include "../modulators/adsr.hpp"
#include "../modulators/attenuator.hpp"
#include "../modulators/macros.hpp"

typedef std::map <uint32_t, std::tuple< std::vector <std::unique_ptr<Input> >*,
    std::vector <std::unique_ptr<Output> >*> > IO_container;


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


class ProcessorMatrix
{
public:
    ProcessorMatrix ();

    uint32_t add_processor (uint8_t type);
    void remove_processor (uint32_t id);
    Processor* get_processor (uint32_t id);


    void set_output_node(uint32_t num);
    void set_SR(double sample_rate);
    void set_lock();


    bool plug_internal(uint32_t src, uint32_t dest, uint16_t src_out, uint16_t dest_in);
    void plug_external (Output* out, uint32_t dest_in);
    
    double process ();

    void* serialize ();
    Input* get_in (uint16_t num);

    
    IO_container* get_IO() { return &processors_io; }

    ~ProcessorMatrix ();
private:

    IO_container processors_io;


    std::vector < std::unique_ptr <Input>  > global_inputs;
    std::vector < std::unique_ptr <Output>  > global_outputs;

    std::vector < std::unique_ptr <Processor> > processors;

    double sample_rate = 0;
    int32_t output_node = 0;
    bool is_locked = false;

};


#endif
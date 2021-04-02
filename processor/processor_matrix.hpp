#ifndef APROCESSORMTX_H
#define APROCESSORMTX_H

#include "../generators/wt.hpp"
#include "../modulators/lfo.hpp"
#include "../modulators/adsr.hpp"
#include "../modulators/attenuator.hpp"
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


    void plug_internal(uint32_t src, uint32_t dest, uint16_t src_out, uint16_t dest_in);

    void plug_external (Output* out, uint32_t dest_in);
    void set_output_node(uint32_t num);

    double process ();
    void set_SR(double sample_rate);
    
    void* serialize ();
    Input* get_in (uint16_t num);



    ~ProcessorMatrix ();
private:
    std::map <uint32_t, std::tuple< std::vector <std::unique_ptr<Input> >*,
        std::vector <std::unique_ptr<Output> >*> > processors_io;

    std::vector < std::unique_ptr <Input>  > global_inputs;
    std::vector < std::unique_ptr <Output>  > global_outputs;

    std::vector < std::unique_ptr <Processor> > processors;

    uint32_t output_node = 0;

};


#endif
#ifndef APROCESSORMTX_H
#define APROCESSORMTX

#include "../generators/wt.hpp"
#include "../modulators/lfo.hpp"
#include "../modulators/adsr.hpp"

enum kMtxInputs
{
    kMtxGate,
    kMtxNote,
    kMtxPitchWheel,
    kMtxVeloAmp
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
    void* get_processor (uint32_t id);


    void create_route (uint32_t src, uint32_t dest, uint16_t src_out, uint16_t dest_in);
    void plug_external(Output* out, uint32_t dest_in);

    void process ();
    void set_SR(double sample_rate);

    void* serialize ();
    Input* get_in ();

    ~ProcessorMatrix ();
private:
    std::map <uint32_t, std::tuple< std::vector <std::unique_ptr<Input> >*,
        std::vector <std::unique_ptr<Output> >*> > processors_io;

    std::vector < std::unique_ptr <Input>  > global_inputs;
    std::vector < std::unique_ptr <Output>  > global_outputs;
    std::vector < std::unique_ptr <Processor> > processors;

};


#endif
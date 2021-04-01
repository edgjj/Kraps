#ifndef WT_H
#define WT_H
#include <array>
#include "generator.hpp"

#define NUM_OCTAVES 11

enum kWtInputs 
{
    kWtShiftIn = kGenPhaseIn + 1
};

class Wavetable : public Generator{
public:

    Wavetable(uint16_t);

    void fill_table_from_buffer (float*, uint32_t);
    void fill_table_from_fcn (double (*fcn) (double phase));
    void set_shift (uint32_t);
    

    ~Wavetable ();

protected:
    void process_callback () override;
    void process_params () override { ; }
private:
    void fill_mipmap ();
    void alloc_dft ();

    
    double phase_cst = 0.0;



    uint16_t waveform_size = 0;
    uint32_t table_size = 0;
    uint32_t shift = 0;

    double* table = nullptr;
    std::array <double*, NUM_OCTAVES> tables { nullptr };

    double* dft = nullptr;
    int* ip = nullptr;
    double* w = nullptr;
};

#endif 
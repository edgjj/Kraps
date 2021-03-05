#ifndef WT_H
#define WT_H
#include <array>
#include "generator.hpp"

#define NUM_OCTAVES 11

class Wavetable : public Generator{
public:

    Wavetable(double, uint16_t);

    void fill_table_from_buffer (float*, uint32_t);
    void fill_table_from_fcn (double (*fcn) (double phase));
    void set_shift (uint32_t);

    void process () override;

    ~Wavetable ();
private:
    void fill_mipmap ();
    void alloc_dft ();

    uint16_t waveform_size;
    uint32_t table_size = 0;
    uint32_t shift = 0;

    double* table = nullptr;
    std::array <double*, NUM_OCTAVES> tables { nullptr };

    double* dft = nullptr;
    int* ip = nullptr;
    double* w = nullptr;
};

#endif 
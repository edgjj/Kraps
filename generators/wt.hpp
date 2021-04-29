#ifndef WT_H
#define WT_H
#include <array>
#include "generator.hpp"
#include "../fft/kissfft.hpp"

#define NUM_OCTAVES 12

namespace kraps {

enum kWtInputs 
{
    kWtShiftIn = kGenGate + 1
};


class Wavetable : public Generator{
public:

    Wavetable(uint16_t);

    void fill_table_from_buffer (float*, uint32_t);
    void fill_table_from_buffer (double*, uint32_t);

    void fill_table_from_fcn (double (*fcn) (double phase));
    nlohmann::json get_serialize_obj() override;
    void set_serialize(nlohmann::json) override;
    double* get_table_view () const;
    uint32_t get_shift();
    uint16_t get_wform_size();
    uint32_t get_table_size();
    ~Wavetable ();

protected:
    void process_callback () override;
    void process_params() override;
private:
    void fill_mipmap ();

    uint32_t shift = 0;
    double phase_cst = 0.0;

    uint16_t waveform_size = 0;
    uint32_t table_size = 0;

    std::unique_ptr<double[]> table;
    std::array <std::unique_ptr<double[]>, NUM_OCTAVES> tables;

};

}

#endif 
#ifndef WT_H
#define WT_H
#include <array>
#include "generator.hpp"

#define NUM_OCTAVES 11

namespace kraps {

enum kWtInputs 
{
    kWtShiftIn = kGenPhaseIn + 1
};


class Wavetable : public Generator{
public:

    Wavetable(uint16_t);

    template <typename T>
    void fill_table_from_buffer (T*, uint32_t);

    void fill_table_from_fcn (double (*fcn) (double phase));
    nlohmann::json get_serialize_obj() override;
    void set_serialize(nlohmann::json) override;


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

    std::unique_ptr<double[]> table;
    std::array <std::unique_ptr<double[]>, NUM_OCTAVES> tables;

    std::unique_ptr<double[]> dft;
    std::unique_ptr<int[]> ip;
    std::unique_ptr<double[]> w;
};

}

#endif 
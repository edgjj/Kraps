#include <memory>
#include <cmath>
#include <cstring>

#include "wt.hpp"

extern "C" 
{
    void rdft(int, int, double *, int *, double *);
}

namespace kraps {

Wavetable::Wavetable(uint16_t waveform_size) : Generator (p_wt, 1, 0)
{
    this->waveform_size = waveform_size;
    phase_cst = this->waveform_size / ( 2.0 * M_PI );
}

Wavetable::~Wavetable()
{

    
}


void Wavetable::process_callback()
{
    if (SR_cst == 0.0 || inputs[kGenFreqIn]->src->val == 0.0)
        return;

    set_freq();

    double phase_cvt    = phase_cst * phase ;

    unsigned int pos_int    = *inputs[kWtShiftIn] + phase_cvt;
    double pos_frac     = phase_cvt - (int)phase_cvt;
    
    double num_oct      = log2 (freq * table_size * SR_cst);
    
    unsigned int no_strip   = (unsigned int)num_oct;
    double  oct_frac    = num_oct - no_strip;

    double o1 = tables[no_strip][pos_int + 1] * pos_frac + tables[no_strip][pos_int] * (1 - pos_frac);
    double o2 = tables[no_strip + 1][pos_int + 1] * pos_frac + tables[no_strip + 1][pos_int] * (1 - pos_frac);

    *outputs[kGenAudioOut] = o1 * (1 - oct_frac) + o2 * oct_frac;
    *outputs[kGenPhaseOut] = phase;

    inc_phase ();
}

void Wavetable::fill_table_from_buffer (float* buf, uint32_t len)
{
    WAIT_LOCK


    table.reset(new double[len]);

    table_size = len;

    for (int i = 0; i < table_size; i++)
        table[i] = buf[i];

    fill_mipmap();
    

}

void Wavetable::fill_table_from_fcn (double (*fcn) (double phase))
{
    WAIT_LOCK

    table.reset(new double[waveform_size]);

    table_size = waveform_size;

    for (int i = 0; i < table_size; i++)
		table[i] = (*fcn) ( 2 * M_PI * ( (float) i  / waveform_size ) );
	
     
    
    fill_mipmap();

}


inline void Wavetable::alloc_dft ()
{
    dft.reset   (new double[table_size]);
    ip.reset    (new int[sqrt(table_size) + 2]);
    ip[0]       = 0;
    w.reset     (new double[table_size - 1]);
}


void Wavetable::fill_mipmap ()
{

    alloc_dft();

    int32_t wt_sz = table_size;
    for (int i = 0; i < NUM_OCTAVES; i++){
        
        tables[i].reset(new double[table_size]);
        std::memcpy (tables[i].get(), table.get(), table_size*sizeof(double));

        rdft (wt_sz,1,tables[i].get(),ip.get(),w.get());

        uint16_t mid    = wt_sz / 2;
        uint16_t bins   = wt_sz / pow(2,i);

        for (uint16_t j = bins; j < wt_sz-1; j = j+2){
            tables[i][j]    = 0.f;
            tables[i][j+1]  = 0.f;
        }


        rdft(wt_sz,-1,tables[i].get(),ip.get(),w.get());
        
        for (int j = 0; j <= wt_sz - 1; j++) {
                tables[i][j] *= 2.0f / wt_sz;
        }

    } 
}

}
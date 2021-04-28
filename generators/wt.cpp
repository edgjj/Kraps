#include <memory>
#include <cmath>
#include <cstring>

#include "wt.hpp"
#include "../serialize/base64/base64.hpp"

extern "C" 
{
    void rdft(int, int, double *, int *, double *);
}

namespace kraps {

Wavetable::Wavetable(uint16_t waveform_size) : Generator (p_wt, 1, 0)
{
    this->waveform_size = waveform_size;
    phase_cst = this->waveform_size / ( 2.0 * M_PI );


    io_description[0].push_back({ kWtShiftIn, "SHIFT", "Shifts WT position forward" });
}

Wavetable::~Wavetable()
{
    
}


void Wavetable::process_callback()
{
    if (SR_cst == 0.0 || inputs[kGenFreqIn]->src->val == 0.0)
        return;

    set_freq();

    double phase_cvt    = phase_cst * phase;

    unsigned int pos_int    = *inputs[kWtShiftIn] + phase_cvt;
    unsigned int pos_int_inc = pos_int + 1;

    if (pos_int_inc == waveform_size)
        pos_int_inc = 0;
    double pos_frac     = phase_cvt - (int)phase_cvt;
    double log_arg      = freq * 2 * waveform_size / (44100.0); // lets think every wt we get is 44100
    double num_oct      = 0.0;

    if (log_arg >= 1.0)
        num_oct = log2(log_arg);   
    
    if (num_oct > NUM_OCTAVES - 2)
        num_oct = NUM_OCTAVES - 2;
    // there's somewhere zero-crossings click problem
    // i think it's zero-division related this since we get inf output

    unsigned int no_strip   = (unsigned int)num_oct;
    double  oct_frac    = num_oct - no_strip;

    double o1 = tables[no_strip][pos_int_inc] * pos_frac + tables[no_strip][pos_int] * (1 - pos_frac);
    double o2 = tables[no_strip + 1][pos_int_inc] * pos_frac + tables[no_strip + 1][pos_int] * (1 - pos_frac);

    *outputs[kGenAudioOut] = o1 * (1 - oct_frac) + o2 * oct_frac;
    *outputs[kGenPhaseOut] = phase;

    inc_phase ();
}

template <typename T>
void Wavetable::fill_table_from_buffer (T* buf, uint32_t len)
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


void Wavetable::fill_mipmap () // incorrect too
{

    alloc_dft();

    int32_t wt_sz = table_size;
    for (int i = 0; i < NUM_OCTAVES; i++){
        
        tables[i].reset(new double[table_size]);
        std::memcpy (tables[i].get(), table.get(), table_size*sizeof(double));

        rdft (wt_sz,1,tables[i].get(),ip.get(),w.get());

        //uint16_t mid    = wt_sz / 2;
        uint16_t bins   = waveform_size / pow(2,i); // 2048 - 1024 - 512 - 256 - 128 - 64 - 32 - 16 - 8 - 4 - 2 - 1
        if (bins == 1) bins = 0;
        for (uint16_t j = bins + 2; j < wt_sz-1; j = j+2){
            tables[i][j]    = 0.f;
            tables[i][j+1]  = 0.f;
        }

        rdft(wt_sz,-1,tables[i].get(),ip.get(),w.get());
        
        for (int j = 0; j <= wt_sz - 1; j++) {
                tables[i][j] *= 2.0f / wt_sz;
        }

    } 
}

nlohmann::json Wavetable::get_serialize_obj()
{
    nlohmann::json o;
    o["table"] = base64_encode((BYTE*) table.get(), sizeof(double) * table_size);
    o["table_size"] = table_size;
    o["waveform_size"] = waveform_size;
    o.update(Processor::get_serialize_obj());

    return o;
}

void Wavetable::set_serialize(nlohmann::json obj)
{
    Processor::set_serialize(obj);
    if (obj.find("table_size") != obj.end())
        obj["table_size"].get_to(table_size);

    if (obj.find("waveform_size") != obj.end())
        obj["waveform_size"].get_to(table_size);

    if (obj.find("table") != obj.end())
    {
        auto decoded = base64_decode(obj["table"]);
        fill_table_from_buffer((double*) decoded.data(), table_size);
    }
}

}
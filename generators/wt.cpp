#include <memory>
#include <cmath>
#include <cstring>

#include "wt.hpp"

extern "C" 
{
    void rdft(int, int, double *, int *, double *);
}

Wavetable::Wavetable(double SR, uint16_t waveform_size) : Generator (SR)
{
    this->waveform_size = waveform_size;
}

Wavetable::~Wavetable()
{
    free(dft);
    free(ip);
    free(w);
    free(table);
    for (int i = 0; i < NUM_OCTAVES; i++)
        free(tables[i]);
}


void Wavetable::process()
{
    uint32_t n = waveform_size;
    int32_t wt_s = table_size;

    int32_t pos_int = (int)phase + shift; 
    float pos_frac = phase - (int)phase;
    
    float num_oct = log2(freq * table_size * SR_cst);
    
    uint16_t num_oct_strp = (uint16_t)num_oct;
    float oct_frac = num_oct - num_oct_strp;
    //oct_frac = tanhf(2*oct_frac);

    double o1 = tables[num_oct_strp][pos_int + 1] * pos_frac + tables[num_oct_strp][pos_int] * (1 - pos_frac);
    double o2 = tables[num_oct_strp+1][pos_int + 1] * pos_frac + tables[num_oct_strp+1][pos_int] * (1 - pos_frac);

    double out = o1 * (1 - oct_frac) + o2 * oct_frac;
    
    set_sample(out, 0);
    inc_phase();
}

void Wavetable::fill_table_from_buffer (float* buf, uint32_t len)
{
    if (table == nullptr)
    {
        table = (double*) malloc(len*sizeof(double));
    } 
    else 
    {
        table = (double*) realloc(table, len*sizeof(double));
    }

    table_size = len;

    for (int i = 0; i < table_size; i++)
        table[i] = buf[i];

    fill_mipmap();
    
    LOG::info ("Filled WT: " + std::to_string(len));

}

void Wavetable::fill_table_from_fcn (double (*fcn) (double phase))
{
    if (table == nullptr)
    {
        table = (double*) malloc(waveform_size*sizeof(double));
    } 
    else 
    {
        table = (double*) realloc(table, waveform_size*sizeof(double));
    }

    for (int i = 0; i < table_size; i++){
		table[i] = (*fcn) ( (float) i / 2048 );
	}
    
    table_size = waveform_size;
    
    fill_mipmap();

	LOG::info("Filled WT (math fcn.)");
}

void Wavetable::set_shift (uint32_t shift)
{
    this->shift = shift;
}

inline void Wavetable::alloc_dft ()
{
    dft = (double*) malloc(sizeof(double)*table_size);
    ip = (int*) malloc((sqrt(table_size)+2)*sizeof(int));
    ip[0] = 0;
    w = (double*) malloc((table_size-1)*sizeof(double));
}



void Wavetable::fill_mipmap ()
{
    float base_note = 11.562f;

    alloc_dft();

    int32_t wt_sz = table_size;
    for (int i = 0; i < NUM_OCTAVES; i++){
        
        tables[i] = (double*) malloc(table_size*sizeof(double));
        std::memcpy(tables[i], table, table_size*sizeof(double));

        rdft(wt_sz,1,tables[i],ip,w);

        uint16_t mid = wt_sz/2;
        uint16_t bins = wt_sz / pow(2,i);

        for (uint16_t j = bins; j < wt_sz-1; j = j+2){
            tables[i][j] = 0.f;
            tables[i][j+1] = 0.f;
        }


        rdft(wt_sz,-1,tables[i],ip,w);
        
        for (int j = 0; j <= wt_sz - 1; j++) {
                tables[i][j] *= 2.0f / wt_sz;
        }
        LOG::info("Passed octave: " + std::to_string(i) + "; Bins filtered " + std::to_string(bins));
    } 
}
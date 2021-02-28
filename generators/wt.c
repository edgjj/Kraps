
#include "../misc/misc.h"
#include "wt.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

void rdft(int, int, double *, int *, double *);
#define USE_CDFT_PTHREADS
#define NUM_OCTAVES 11


void wt_setup(WTable* wt, float sample_rate){

    wt->phase = 0.f;

    assert(sample_rate > 0);

	wt->SR = sample_rate;
	wt->waveform_size = 2048;
    wt->table = NULL;
    wt->tables = malloc(NUM_OCTAVES * sizeof(double*));
    wt->table_size = 0;

    

    wt->dft = NULL;
    wt->ip = NULL;
    wt->w = NULL;

    DEBUG_PRINT("Set up generator. Waveform size: %u\n",wt->waveform_size);
	
}

void wt_alloc_dft(WTable* wt){
    wt->dft = malloc(sizeof(double)*wt->table_size);
    wt->ip = malloc((sqrt(wt->table_size)+2)*sizeof(int));
    wt->ip[0] = 0;
    wt->w = malloc((wt->table_size-1)*sizeof(double));
}

void wt_free_dft(WTable* wt){
    free(wt->dft);
    free(wt->ip);
    free(wt->w);
}

void wt_fill_mipmap(WTable* wt){
    float base_note = 11.562f;

    wt_alloc_dft(wt);
    int32_t wt_sz = wt->table_size;
    for (int i = 0; i < NUM_OCTAVES; i++){
        
        wt->tables[i] = malloc(wt->table_size*sizeof(double));
        memcpy(wt->tables[i],wt->table,wt->table_size*sizeof(double));

        rdft(wt_sz,1,wt->tables[i],wt->ip,wt->w);

        uint16_t mid = wt_sz/2;
        uint16_t bins = wt_sz / pow(2,i);

        for (uint16_t j = bins; j < wt_sz-1; j = j+2){
            wt->tables[i][j] = 0.f;
            wt->tables[i][j+1] = 0.f;
        }


        rdft(wt_sz,-1,wt->tables[i],wt->ip,wt->w);
        
        for (int j = 0; j <= wt_sz - 1; j++) {
                wt->tables[i][j] *= 2.0f / wt_sz;
        }
        DEBUG_PRINT("Passed octave: %d; Bins filtered %d  \n",i, bins);
    } 
    
}

void wt_fill_table(WTable* wt, float* buf, uint32_t len){
    
    if (wt->table == NULL){
        wt->table = malloc(len*sizeof(double));
    } else {
        wt->table = realloc(wt->table, len*sizeof(double));
    }
    wt->table_size = len;
    for (int i = 0; i < wt->table_size;i++)
        wt->table[i] = buf[i];

    wt_fill_mipmap(wt);
    
	DEBUG_PRINT("Filled WT: %d\n", len);

}



void wt_fill_table_from_fcn(WTable* wt, float (*fcn)(float phase)){

    if (wt->table == NULL){
        wt->table = malloc(wt->waveform_size*sizeof(double));
    } else {
        wt->table = realloc(wt->table, wt->waveform_size*sizeof(double));
    }

    for (int i = 0; i < wt->table_size; i++){
		wt->table[i] = (*fcn) ( (float) i / 2048 );
	}
    
    wt->table_size = wt->waveform_size;
    
    wt_fill_mipmap(wt);

	DEBUG_PRINT("Filled WT (math fcn.)\n", wt->waveform_size);

}

float wt_get_sample(WTable* wt, float freq, uint32_t shift){
    // base tune is ~= F-1

    uint32_t n = wt->waveform_size;
    int32_t wt_s = wt->table_size;

    
    float step = freq * n / wt->SR;
    float r_phase = wt->phase + step;
    

    int32_t pos_int = (int)wt->phase + shift; 
    float pos_frac = wt->phase - (int)wt->phase;
    
    float num_oct = log2(freq * wt->table_size / wt->SR);
    uint16_t num_oct_strp = (uint16_t)num_oct;
    float oct_frac = num_oct - num_oct_strp;
    

    float o1 = wt->tables[num_oct_strp][pos_int + 1] * pos_frac + wt->tables[num_oct_strp][pos_int] * (1 - pos_frac);
    float o2 = wt->tables[num_oct_strp+1][pos_int + 1] * pos_frac + wt->tables[num_oct_strp+1][pos_int] * (1 - pos_frac);

    float out = o1 * (1 - oct_frac) + o2 * oct_frac;
    

    while (r_phase > 2048.f)
		r_phase -= 2048.f;

	wt->phase = r_phase;

	return out;
}

void wt_clear(WTable* wt){
    free(wt->table);
    free(wt->dft);
    wt->dft = NULL;
    wt->table = NULL;
}


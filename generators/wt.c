
#include "../misc/misc.h"
#include "wt.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

void rdft(int, int, double *, int *, double *);
#define USE_CDFT_PTHREADS

void wt_setup(WTable* wt, float sample_rate){

    wt->phase = 0.f;

    assert(sample_rate > 0);

	wt->SR = sample_rate;
	wt->waveform_size = 2048;
    wt->table = NULL;
    wt->table_size = 0;

    wt->dft = malloc(sizeof(double)*wt->waveform_size);
    wt->ip = malloc((sqrt(wt->waveform_size)+2)*sizeof(int));
    wt->w = malloc((wt->waveform_size-1)*sizeof(double));
    wt->ip[0] = 0;

    DEBUG_PRINT("Set up generator. Waveform size: %u\n",wt->waveform_size);
	

}
void wt_fill_table(WTable* wt, float* buf, uint32_t len){

    if (wt->table == NULL){
        wt->table = malloc(len*sizeof(double));
    } else {
        wt->table = realloc(wt->table, len*sizeof(double));
    }
    
    for (int i = 0; i < len; i++){
		wt->table[i] = buf[i];
	}
    wt->table_size = len;

	DEBUG_PRINT("Filled WT: %f\n", len);

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

	DEBUG_PRINT("Filled WT (math fcn.)\n", wt->waveform_size);
}
void test_dft(WTable* wt){

    memcpy(wt->dft,
        wt->table,
        sizeof(double) * wt->waveform_size);
    
	rdft(wt->waveform_size,1,wt->dft,wt->ip,wt->w);

    

    FILE* fp;
    fp = fopen("./dft.txt","w+");
    for (int i = 0; i < wt->waveform_size / 2; i++){    
        fprintf(fp,"%.12f\n",sqrt((pow(wt->dft[2*i],2) + pow(wt->dft[2*i+1],2))/wt->waveform_size));
    }
    
    fclose(fp);
}
float wt_get_sample(WTable* wt, float freq, uint32_t shift){

    uint16_t n = wt->waveform_size;
    assert ( wt->table != NULL || wt->dft != NULL);
    assert ( shift+n <= wt->table_size - 3);
    
    float step = freq * n / wt->SR;
    float r_phase = wt->phase + step ;


    int32_t pos_int = (int)wt->phase; // + shift;
    float pos_frac = wt->phase - (int)wt->phase;
    
    memcpy(wt->dft,
        wt->table + shift * sizeof(double),
        sizeof(double) * n);
    


	rdft(n,1,wt->dft,wt->ip,wt->w);

    int mid = n/2;
    int bins = (1/2 - freq*8/wt->SR)*n;
    for (int i = mid - bins + 1; i < mid + bins + 1; i++){
        wt->dft[i] = 0;
        wt->dft[i+1] = 0;
    }

    rdft(n,-1,wt->dft,wt->ip,wt->w);
    for (int j = 0; j <= n - 1; j++) {
                wt->dft[j] *= 2.0 / n;
            }
    /*
        Algorithm: 
        1. Do RDFT of waveform;
        2. Cut everything from Fnyq - Fplay to Fnyq;
        3. Do inverse RDFT of waveform;
        4. Linear interp. and pop;
    */

   /*

        mid = Nfft/2;
        freq = 15000;
        nACBin = ceil ((fs/2 - freq)*Nfft/fs);

        fftr([mid - nACBin + 1:mid+nACBin + 1]) = 0; % guarantee symmetry; just rectangle window;

        invert = ifft(fftr);
   
   */

    //float out = 0.f;
	float out = wt->dft[pos_int + 1] * pos_frac + wt->dft[pos_int] * (1 - pos_frac);
    

    while (r_phase > 2047.f)
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


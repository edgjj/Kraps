#ifndef WT_H
#define WT_H
#include <stdint.h>


typedef struct WTable{

	float SR;
	float phase;
	

	float phase_inc;
	float freq;

    uint16_t waveform_size;
	uint32_t table_size;
	double *table;
	double **tables;

	/* ooura fft related */
	
	double *dft;
	int* ip;
	double* w;
} WTable;

void wt_setup(WTable* wt, float sample_rate);

void wt_fill_table(WTable* wt, float* buf, uint32_t len);
void wt_fill_table_from_fcn(WTable* wt, float (*fcn)(float phase));
double wt_get_sample(WTable* wt, uint32_t shift);
void wt_set_freq(WTable* wt, float freq);
void wt_clear(WTable* wt);
void test_dft(WTable* wt);


#endif
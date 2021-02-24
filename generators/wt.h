#ifndef WT_H
#define WT_H
#include <stdint.h>


typedef struct WTable{
	float phase;
	float SR;
    uint16_t waveform_size;
	uint32_t table_size;
	double *table;

	// ooura fft
	double *dft;
	int* ip;
	double* w;
} WTable;

void wt_setup(WTable* wt, float sample_rate);
void wt_fill_table(WTable* wt, float* buf, uint32_t len);
void wt_fill_table_from_fcn(WTable* wt, float (*fcn)(float phase));
float wt_get_sample(WTable* wt, float freq, uint32_t shift);
void wt_clear(WTable* wt);
void test_dft(WTable* wt);


#endif
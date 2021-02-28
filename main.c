#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "misc/misc.h"
#include "tinywav/tinywav.h"
#include "modulators/adsr.h"
#include "generators/wt.h"




int main(int argc, char* argv[]){
	
	if (argc < 3){
		printf("Missing args. gen $freq $amp\n");
		return 0;
	}

	float freq = atof(argv[1]), amp = atof(argv[2]);
	printf("Gen. params: %d Hz, %f amp\n",atoi(argv[1]),atof(argv[2]));
	int time = 5, SR = 44100;

	TinyWav tw;	
	ADSR adsr;
	WTable wt;

	
	
	if (tinywav_open_read(&tw,"./m2_wt.wav",TW_INLINE,TW_FLOAT32) != 0){
		printf("Can't read WT\n");
		return -1;
	}

	float wt_read [4096];
	tinywav_read_f(&tw,wt_read,4096);
	tinywav_close_read(&tw);
	
	wt_setup(&wt,SR);
	wt_fill_table(&wt,wt_read,4096);
	
	if (tinywav_open_write(&tw,1,SR,TW_INT16,TW_INLINE,"./generator_test.wav") != 0){
		printf("Can't open file for write\n");
		return -1;
	}

	int numsmp = time*SR;
	float* samples = (float*) malloc (numsmp*sizeof(float));

	adsr_setup(&adsr,SR);
	adsr_set_sustain_amp(&adsr,0.f);
	adsr_set_attack_time(&adsr,0.5f);
	adsr_set_decay_time(&adsr,1.f);
	adsr_set_release_time(&adsr,2.f);


	float check = numsmp/2;
	float freqstep = 680.f/numsmp;
	DEBUG_PRINT("freq_step: %f\n",freqstep);
	
	adsr_gate_on(&adsr);
	//test_dft(&wt);

	clock_t s_time = clock();

	for (int i = 0; i < numsmp; i++){

		samples[i] = adsr_get_coeff(&adsr)*wt_get_sample(&wt,freq,0);
		freq += freqstep;
		if (adsr.gate == 1 && i > check)
			adsr_gate_off(&adsr);
	}
	
	float elapsed = (clock() - s_time)  / (CLOCKS_PER_SEC / 1000);
	printf("Time elapsed: %.10f\n",elapsed);
	tinywav_write_f(&tw,samples,numsmp);

	tinywav_close_write(&tw);
	
	
	int16_t v = 0;
	
	
	printf("Wrote audio data\n");
	
	wt_clear(&wt);
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <memory>

#include "misc/misc.hpp"
#include "tinywav/tinywav.h"
#include "generators/wt.hpp"


int main(int argc, char* argv[]){

	if (argc < 3){
		printf("Missing args. gen $freq $amp\n");
		return 0;
	}

	float freq = atof(argv[1]), amp = atof(argv[2]);
	printf("Gen. params: %d Hz, %f amp\n",atoi(argv[1]),atof(argv[2]));
	int time = 2, SR = 44100;

	TinyWav tw ;	
	auto wt = std::make_unique <Wavetable> (44100.0,2048);
	
	if (tinywav_open_read(&tw,"./m2_wt.wav",TW_INLINE,TW_FLOAT32) != 0){
		printf("Can't read WT\n");
		return -1;
	}

	float wt_read [4096];
	tinywav_read_f(&tw,wt_read,4096);
	tinywav_close_read(&tw);
	
	wt->fill_table_from_buffer (wt_read, 4096);



	int numsmp = time*SR;
	

	float check = numsmp/2;
	float freqstep = 600.f/numsmp;
	LOG::info("freq_step: " + std::to_string(freqstep));


	if (tinywav_open_write(&tw,1,SR,TW_INT16,TW_INLINE,"./generator_tes.wav") != 0){
		printf("Can't open file for write\n");	
		return -1;
	}

	float* samples = (float*) malloc (numsmp*sizeof(float));

	for (int i = 0; i < numsmp; i++){
		wt->set_freq(freq);
        wt->process();
		samples[i] = wt->get_sample(0);
		
		/* pblep_set_freq(&pblep,freq);
		samples[i] = adsr_get_coeff(&adsr)*amp*pblep_get_sample(&pblep); */

		freq += freqstep;
		
		/* if (adsr.gate == 1 && i > check)
			adsr_gate_off(&adsr); */
	}
	

	tinywav_write_f(&tw,samples,numsmp);
	tinywav_close_write(&tw);
	
	
	int16_t v = 0;
	
	
	printf("Wrote audio data\n");

	return 0;
}

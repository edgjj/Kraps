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
#include "modulators/adsr.hpp"

int main(int argc, char* argv[]){

	if (argc < 3){
		printf("Missing args. gen $freq $amp\n");
		return 0;
	}

	double freq = atof(argv[1]), amp = atof(argv[2]);
	printf("Gen. params: %d Hz, %f amp\n",atoi(argv[1]),atof(argv[2]));
	int time = 2, SR = 44100;

	TinyWav tw;	
    std::vector <std::unique_ptr <Processor>> processors;

    processors.emplace_back (std::make_unique <Wavetable> (2048));
    processors.emplace_back (std::make_unique <ADSR> ());

	if (tinywav_open_read(&tw,"./m2_wt.wav",TW_INLINE,TW_FLOAT32) != 0){
		printf("Can't read WT\n");
		return -1;
	}

	float wt_read [4096];
	tinywav_read_f(&tw,wt_read,4096);
	tinywav_close_read(&tw);
	
	( (Wavetable*) processors[0].get() )->fill_table_from_buffer (wt_read, 4096);

    std::unique_ptr <Output> freq_out;
    freq_out.reset (new Output(NULL));
    *freq_out = freq;

	std::unique_ptr <Output> shift_out;
    shift_out.reset (new Output(NULL));
	*shift_out = 0;


	std::unique_ptr <Output> gate_out;
    gate_out.reset (new Output(NULL));
	*gate_out = true;

	std::unique_ptr <Input> fin_out;
    fin_out.reset (new Input(NULL, processors[1]->get_out(kADSRAudioOut)));


	int numsmp = time*SR;
    processors[0]->plug (freq_out.get(), kGenFreqIn);
	processors[0]->plug (shift_out.get(), kWtShiftIn);
    processors[1]->plug (processors[0].get(), kGenAudioOut, kADSRAudioIn);
	processors[1]->plug (gate_out.get(), kADSRGate);


	for (auto& i : processors)
		i->set_SR (SR);

	float check = numsmp/2;
	float freqstep = 600.f/numsmp;
	LOG::info("freq_step: " + std::to_string(freqstep));


	if (tinywav_open_write(&tw,1,SR,TW_INT16,TW_INLINE,"./generator_tes.wav") != 0){
		printf("Can't open file for write\n");	
		return -1;
	}

	float* samples = (float*) malloc (numsmp*sizeof(float));


	for (int i = 0; i < numsmp; i++){

        for (auto &i : processors)
            i->process();
        
		samples[i] = *fin_out;
		
		*freq_out += freqstep;
		
		if (i > check)
			*gate_out = false;
			
	}
	

	tinywav_write_f(&tw,samples,numsmp);
	tinywav_close_write(&tw);
	
	
	int16_t v = 0;
	
	
	printf("Wrote audio data\n");

	return 0;
}

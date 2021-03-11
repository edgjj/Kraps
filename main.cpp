#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <memory>
#include <chrono>

#include "misc/misc.hpp"
#include "tinywav/tinywav.h"
#include "generators/wt.hpp"
#include "modulators/adsr.hpp"
#include "modulators/lfo.hpp"
#include "modulators/attenuator.hpp"

int main(int argc, char* argv[]){

	if (argc < 3){
		printf("Missing args. gen $freq $amp\n");
		return 0;
	}

	double freq = atof(argv[1]), amp = atof(argv[2]);
	printf("Gen. params: %d Hz, %f amp\n",atoi(argv[1]),atof(argv[2]));
	int time = 2, SR = 44100;

	
	TinyWav tw;	

	if (tinywav_open_read(&tw,"./m2_wt.wav",TW_INLINE,TW_FLOAT32) != 0){
		printf("Can't read WT\n");
		return -1;
	}

	float wt_read [4096];
	tinywav_read_f(&tw,wt_read,4096);
	tinywav_close_read(&tw);

    std::vector <std::unique_ptr <Processor>> processors;

    processors.emplace_back (std::make_unique <Wavetable> (2048));
    processors.emplace_back (std::make_unique <ADSR> ());
	processors.emplace_back (std::make_unique <LFO> ());
	processors.emplace_back (std::make_unique <Attenuator> ());

	
    std::unique_ptr <Output> freq_out;
    freq_out.reset (new Output(nullptr));
    *freq_out = freq;

	std::unique_ptr <Output> lfo_freq;
    lfo_freq.reset (new Output(nullptr));
    *lfo_freq = 1;

	std::unique_ptr <Output> shift_out;
    shift_out.reset (new Output(nullptr));
	*shift_out = 0;


	std::unique_ptr <Output> gate_out;
    gate_out.reset (new Output(nullptr));
	*gate_out = true;

	std::unique_ptr <Input> fin_out;
    fin_out.reset (new Input(nullptr, processors[3]->get_out(kAttenOut)));


	int numsmp = time*SR;
    processors[0]->plug (freq_out.get(), kGenFreqIn);
	processors[0]->plug (shift_out.get(), kWtShiftIn);
	processors[1]->plug (gate_out.get(), kADSRGate);
	processors[2]->plug (lfo_freq.get(), kLFOFreqIn);

	processors[3]->plug (processors[0].get(), kGenAudioOut, kAttenIn);
	processors[3]->plug (processors[2].get(), kLFOAudioOut, kAttenMod);

	( (Wavetable*) processors[0].get() )->fill_table_from_buffer (wt_read, 4096);

	processors[1]->set_param (adsr_attack, 0.3);
	processors[1]->set_param (adsr_decay, 0.1);
	processors[1]->set_param (adsr_sustain, -6);
	processors[1]->set_param (adsr_release, 0.5);

	LFO* lfo_iface = (LFO*) processors[2].get();

	lfo_iface->add_point ( {0.5, 1.0} );
	lfo_iface->add_point ( {0.3, 0.4} );
	lfo_iface->set_tension (0, -0.7);
	lfo_iface->set_tension (2, 0.89);

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


	using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;

	auto t1 = high_resolution_clock::now();

	for (int i = 0; i < numsmp; i++){

        for (auto &i : processors)
            i->process();
        
		samples[i] = *fin_out;
		
		*freq_out += freqstep;
		
		if (i > check)
			*gate_out = false;
			
	}
	
	auto t2 = high_resolution_clock::now();

	duration<double, std::milli> ms_double = t2 - t1;

	std::cout << "Processing finished. t: " << ms_double.count() 
		<< "; Time per sample: " << ms_double.count() / numsmp << "; Sample count: " << numsmp << std::endl;


	tinywav_write_f(&tw,samples,numsmp);
	tinywav_close_write(&tw);
	free (samples);
	
	int16_t v = 0;
	
	
	printf("Wrote audio data\n");

	
	
	exit (0);
}

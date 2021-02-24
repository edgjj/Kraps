#ifndef SAMPLEFILTER_H_
#define SAMPLEFILTER_H_

/*

FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 44100 Hz

* 0 Hz - 19000 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = 4.085425600894298 dB

* 20000 Hz - 20900 Hz
  gain = 0
  desired attenuation = -160 dB
  actual attenuation = -160.19374770590346 dB

* 21000 Hz - 22050 Hz
  gain = 0
  desired attenuation = -160 dB
  actual attenuation = -160.19374770590346 dB

*/

#define SAMPLEFILTER_TAP_NUM 161

typedef struct {
  double history[SAMPLEFILTER_TAP_NUM];
  unsigned int last_index;
} SampleFilter;

void SampleFilter_init(SampleFilter* f);
void SampleFilter_put(SampleFilter* f, double input);
double SampleFilter_get(SampleFilter* f);

#endif
#ifndef FFTWPLAN_H
#define FFTWPLAN_H

#include <fftw3.h>
#include "pulseaudio_input.h"
#include <complex>

class FftwPlan
{
public:
    FftwPlan(int insz, int outsz);
    ~FftwPlan();
    void pulse_to_in(pulseaudio_input* pls);
    void exec();
    std::complex<double> get_fft(int i);
private:
    int sizeIn, sizeOut;
    double* in;
    fftw_complex *out;
    fftw_plan plan;
};

#endif // FFTWPLAN_H

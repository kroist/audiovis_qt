#include "fftwplan.h"

FftwPlan::FftwPlan(int insz, int outsz) {
    sizeIn = insz;
    sizeOut = outsz;
    in = (double*) fftw_malloc(sizeof(double) * insz);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * outsz);
    plan = fftw_plan_dft_r2c_1d(insz, in, out, FFTW_ESTIMATE);

}

FftwPlan::~FftwPlan() {
    fftw_free(in);
    fftw_free(out);
    fftw_destroy_plan(plan);
}

void FftwPlan::pulse_to_in(pulseaudio_input* pls) {
    int bufsz = pls->buf_size();
    for (int i = 0; i < sizeIn; i++) {
        if (sizeIn < bufsz) {
            in[i] = pls->buf_get((int) (bufsz*(double)i/sizeIn ) ); /* almost windowing function */
            in[i] /= 32766.0; /* rescale amplitude to [0...1] */
        }
        else {
            if (i < bufsz)
                in[i] = pls->buf_get(i)/32766.0;
            else
                in[i] = 0;
        }
    }

}

void FftwPlan::exec() {
    fftw_execute(plan);
}

std::complex<double> FftwPlan::get_fft(int i) {
    return std::complex<double> (out[i][0], out[i][1]);
}

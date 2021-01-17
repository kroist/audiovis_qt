#include "pulseaudio_input.h"
#include <iostream>

pulseaudio_input::pulseaudio_input(int channels, std::string device) {
    /* sample parameters */
    ss.format = PA_SAMPLE_S16LE;
    ss.channels = channels;
    ss.rate = 44100;
    ///device is
    std::cout << device << std::endl;
    s = pa_simple_new(NULL, "fftw", PA_STREAM_RECORD, device.c_str(), "ffting", &ss, NULL, &pb, NULL);
}

pulseaudio_input::~pulseaudio_input() {
    pa_simple_free(s);
}

void pulseaudio_input::read_buf() {
    pa_simple_read(s, this->buf, sizeof(this->buf), NULL);
}

int pulseaudio_input::buf_size() {
    return BUFSZ;
}

int16_t pulseaudio_input::buf[] = {};

int16_t pulseaudio_input::buf_get(int i) {
    return this->buf[i];
}



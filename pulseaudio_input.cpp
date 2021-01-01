#include "pulseaudio_input.h"

pulseaudio_input::pulseaudio_input() {
    /* sample parameters */
    ss.format = PA_SAMPLE_S16LE;
    ss.channels = 1;
    ss.rate = 44100;
    s = pa_simple_new(NULL, "fftw", PA_STREAM_RECORD, "alsa_output.pci-0000_00_1f.3.analog-stereo.monitor", "ffting", &ss, NULL, &pb, NULL);
}

pulseaudio_input::~pulseaudio_input() {
    pa_simple_free(s);
}

void pulseaudio_input::read_buf() {
    pa_simple_read(s, buf, sizeof(buf), NULL);
}

int pulseaudio_input::buf_size() {
    return BUFSZ;
}

int16_t pulseaudio_input::buf_get(int i) {
    return buf[i];
}

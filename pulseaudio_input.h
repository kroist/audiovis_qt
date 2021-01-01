#ifndef PULSEAUDIO_INPUT_H
#define PULSEAUDIO_INPUT_H

#include <pulse/pulseaudio.h>
#include <pulse/simple.h>

class pulseaudio_input
{
public:
    pulseaudio_input();
    ~pulseaudio_input();
    void read_buf();
    int buf_size();
    int16_t buf_get(int i);
private:
    const static int BUFSZ = 2048;
    int16_t buf[BUFSZ];
    pa_simple *s;
    pa_sample_spec ss;
    const pa_buffer_attr pb = {.maxlength = (uint32_t)-1, .fragsize = BUFSZ}; /*without this read from api is too slow */

};

#endif // PULSEAUDIO_INPUT_H

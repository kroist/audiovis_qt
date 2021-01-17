#ifndef PULSEAUDIO_INPUT_H
#define PULSEAUDIO_INPUT_H

#include <pulse/pulseaudio.h>
#include <pulse/simple.h>
#include <pulse/introspect.h>
#include <pulse/context.h>
#include <pulse/mainloop.h>
#include <pulse/mainloop-api.h>
#include <pulse/operation.h>

#include <string>

class pulseaudio_input
{
public:
    pulseaudio_input(int channels, std::string device);
    ~pulseaudio_input();
    void read_buf();
    int buf_size();

    int16_t buf_get(int i);
    const static int BUFSZ = 2048;
    static int16_t buf[BUFSZ];
private:
    std::string device_name;
    int channels;
    pa_simple *s;
    pa_sample_spec ss;
    const pa_buffer_attr pb = {.maxlength = (uint32_t)-1, .fragsize = BUFSZ}; /*without this read from api is too slow */

};

#endif // PULSEAUDIO_INPUT_H

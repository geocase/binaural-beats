#include <stdio.h>
#include <stdbool.h>
#include <AL/al.h>
#include <AL/alc.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "gui.h"

#define MAX_CHANNELS 2

struct AudioPlayer {
    ALCdevice *device;
    ALCcontext *context;
};

typedef struct {
    size_t size;
    uint32_t sample_rate;
    int16_t* buffer;
} AudioSample_t;

static void
list_audio_devices(const ALCchar *devices) {
    const ALCchar *device = devices, *next = devices + 1;
    size_t len = 0;
    printf("devices:\n");
    while (device && *device != '\0' && next && *next != '\0') {
        printf("%s\n", devices);
        len = strlen(device);
        device += (len + 1);
        next += (len + 2);
    }
}

struct AudioPlayer
audioplayer_Init() {
    list_audio_devices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));

    struct AudioPlayer ap;
    ap.device = alcOpenDevice(NULL);
    if (!ap.device) {
        exit(-1);
    }
    ap.context = alcCreateContext(ap.device, NULL);
    if (!alcMakeContextCurrent(ap.context)) {
        exit(-1);
    }
    return ap;
}


AudioSample_t
generateSineWaveSample(float frequency, uint64_t length, uint32_t sample_rate) {
    AudioSample_t ret;
    ret.size = length * sample_rate;
    ret.buffer = malloc(ret.size * sizeof(int16_t));
    ret.sample_rate = sample_rate;

    int scale = (pow(2, 16) / 2) - 1;
    for(int i = 0; i < ret.size; ++i) {
        float x = sinf((2.0f * (float)M_PI * frequency) / sample_rate * i);
        ret.buffer[i] = scale * x;
    }
    return ret;
}

void freeAudioSample(AudioSample_t* as) {
    free(as->buffer);
}

void test_print() {
    printf("HELLO!\n");
}


ALuint left_source;
ALuint right_source;

AudioSample_t right;
AudioSample_t left;

ALuint right_buffer;
ALuint left_buffer;

void toggle_playback(GtkButton* button) {
    unsigned int out;
    alGetSourcei(right_source, AL_SOURCE_STATE, &out);
    if(out == AL_PLAYING) {
        alSourcePause(left_source);
        alSourcePause(right_source);
        gtk_button_set_label(button, "PLAY");
    } else {
        alSourcePlay(left_source);
        alSourcePlay(right_source);
        gtk_button_set_label(button, "PAUSE");
    }
}

void change_pitch_right(GtkHScale* scale) {
    freeAudioSample(&right);
    right = generateSineWaveSample((float)gtk_range_get_value(scale), 2.0, 96000);
    printf("%f\n", gtk_range_get_value(scale));
    alDeleteBuffers(1, &right_buffer);
    alGenBuffers(1, &right_buffer);
    alBufferData(right_buffer, AL_FORMAT_MONO16, right.buffer, right.size, right.sample_rate);
    alSourceStop(right_source);
    alSourcei(right_source, AL_BUFFER, right_buffer);
    alSourcePlay(right_source);
}

void change_pitch_left(GtkHScale* scale) {
    freeAudioSample(&left);
    left = generateSineWaveSample((float)gtk_range_get_value(scale), 2.0, 96000);
    printf("%f\n", gtk_range_get_value(scale));
    alDeleteBuffers(1, &left_buffer);
    alGenBuffers(1, &left_buffer);
    alBufferData(left_buffer, AL_FORMAT_MONO16, left.buffer, left.size, left.sample_rate);
    alSourceStop(left_source);
    alSourcei(left_source, AL_BUFFER, left_buffer);
    alSourcePlay(left_source);
}

int main() {
    struct AudioPlayer ap = audioplayer_Init();

    left = generateSineWaveSample(202.0f, 2, 96000);
    alGenBuffers(1, &left_buffer);
    alBufferData(left_buffer, AL_FORMAT_MONO16, left.buffer, left.size, left.sample_rate);

    right = generateSineWaveSample(200.0f, 2, 96000);
    alGenBuffers(1, &right_buffer);
    alBufferData(right_buffer, AL_FORMAT_MONO16, right.buffer, right.size, right.sample_rate);

    right_source = 0;
    alSource3f(right_source, AL_POSITION, 0, 0, 0);
    alGenSources(1, &right_source);
    alSourcei(right_source, AL_BUFFER, right_buffer);

    alSource3f(right_source, AL_POSITION, 10, 0, 0);

    left_source = 0;
    alSource3f(left_source, AL_POSITION, 0, 0, 0);
    alGenSources(1, &left_source);
    alSourcei(left_source, AL_BUFFER, left_buffer);

    alSource3f(left_source, AL_POSITION, -5, 0, 0);
    alSource3f(right_source, AL_POSITION, 5, 0, 0);

    alSourcef(left_source, AL_GAIN, 1.0f);
    alSourcef(right_source, AL_GAIN, 1.0f);
    alSourcei(left_source, AL_LOOPING, AL_TRUE);
    alSourcei(right_source, AL_LOOPING, AL_TRUE);

    alSourcePlay(right_source);
    alSourcePlay(left_source);

    GtkWidget* window;
    GtkWidget* vbox;
    gtk_init(NULL, NULL);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_vbox_new(TRUE, 1);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget* button = gtk_button_new_with_label("TEST");
    gtk_box_pack_start(GTK_BOX(vbox), button, TRUE, TRUE, 0);
    g_signal_connect(button, "clicked", G_CALLBACK(toggle_playback), NULL);

    GtkWidget* hscale_right = gtk_hscale_new_with_range(20, 1000, 1);
    gtk_scale_set_draw_value(hscale_right, TRUE);
    gtk_widget_set_size_request(hscale_right, 400, -1);
    gtk_range_set_value(hscale_right, 202);
    g_signal_connect(hscale_right, "value-changed", G_CALLBACK(change_pitch_right), NULL);

    gtk_box_pack_start(GTK_BOX(vbox), hscale_right, TRUE, TRUE, 0);


    GtkWidget* hscale_left = gtk_hscale_new_with_range(20, 1000, 1);
    gtk_scale_set_draw_value(hscale_left, TRUE);
    gtk_widget_set_size_request(hscale_left, 400, -1);
    gtk_range_set_value(hscale_left, 202);
    g_signal_connect(hscale_left, "value-changed", G_CALLBACK(change_pitch_left), NULL);

    gtk_box_pack_start(GTK_BOX(vbox), hscale_left, TRUE, TRUE, 0);

    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    gtk_widget_show_all(window);
    gtk_main();

//    alSourcePlay(right_source);
//    alSourcePlay(left_source);
//
//    unsigned int out;
//    do {
//        alGetSourcei(right_source, AL_SOURCE_STATE, &out);
//    } while(out == AL_PLAYING);

    return 0;
}
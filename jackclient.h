#ifndef JACKCLIENT_H
#define JACKCLIENT_H 1

#include <jack/jack.h>

extern unsigned int jc_errno;

enum jackclient_error {
  JC_SUCCESS = 0,
  JC_OPENCLIENT,
  JC_REGMIDI,
  JC_REGAUDIO,
  JC_ACTIVATE
};

typedef struct {
   jack_client_t* client;
   jack_nframes_t sample_rate;
   jack_port_t* output_port;
   jack_port_t* midiin_port;

   void (*note_on) (void* arg, unsigned char, unsigned char);
   void (*note_off) (void* arg, unsigned char);
   void (*sustain) (void* arg, unsigned char);
   float (*next_frame) (void* arg);
   void* callback_arg;
} jc_t;

jc_t* jc_init(char* client_name,
              void (*note_on_callback) (void* arg, unsigned char, unsigned char),
              void (*note_off_callback) (void* arg, unsigned char),
              void (*sustain_callback) (void* arg, unsigned char),
              float (*next_frame_callback) (void* arg),
              void* callback_arg);
void jc_activate(jc_t* jc);
const char* jc_strerror(unsigned long int jc_errno);

#endif

#include <jack/jack.h>
#include <jack/midiport.h>

#include "jackclient.h"

unsigned int jc_errno = 0;

static const char* error_strings[] = {
   [JC_SUCCESS] = "Success.",
   [JC_OPENCLIENT] = "Couldn't open client.",
   [JC_REGMIDI] = "Couldn't register midi input port.",
   [JC_REGAUDIO] = "Couldn't register audio output port."
};


const char* jc_strerror(unsigned long int jc_errno)
{
   if(jc_errno >= sizeof(error_strings)) {
      return "Unknown error.";
   }
   return error_strings[jc_errno];
}


static int process(jack_nframes_t nframes, void* arg)
{
   jc_t* jc = (jc_t*) arg;
   jack_default_audio_sample_t* output_port_buffer =
      (jack_default_audio_sample_t*) jack_port_get_buffer(jc->output_port, nframes);
   void* midiin_port_buffer = jack_port_get_buffer(jc->midiin_port, nframes);
   jack_nframes_t event_index = 0;
   jack_nframes_t event_count = jack_midi_get_event_count(midiin_port_buffer);
   jack_midi_event_t midiin_event;
   if(event_count > 0) {
      jack_midi_event_get(&midiin_event, midiin_port_buffer, event_index);
   }
  
   for(jack_nframes_t i=0; i<nframes; i++) {
      while(event_index < event_count && midiin_event.time <= i) {
         if((midiin_event.buffer[0] & 0xf0) == 0x90 &&
            midiin_event.buffer[2] != 0x00) {
            jc->note_on(jc->callback_arg,
                        midiin_event.buffer[1],
                        midiin_event.buffer[2]);
         }
         else if((midiin_event.buffer[0] & 0xf0) == 0x80 ||
                 ((midiin_event.buffer[0] & 0xf0) == 0x90 &&
                  midiin_event.buffer[2] == 0x00 )) {
            jc->note_off(jc->callback_arg, midiin_event.buffer[1]);
         }
         else if(midiin_event.buffer[0] == 0xb0 &&
                 midiin_event.buffer[1] == 0x40) {
            jc->sustain(jc->callback_arg, midiin_event.buffer[2]);
         }
         event_index++;
         jack_midi_event_get(&midiin_event, midiin_port_buffer, event_index);
      }
      output_port_buffer[i] = jc->next_frame(jc->callback_arg);
   }
   return 0;
}


jc_t* jc_init(char* client_name,
              void (*note_on_callback) (void* arg, unsigned char, unsigned char),
              void (*note_off_callback) (void* arg, unsigned char),
              void (*sustain_callback) (void* arg, unsigned char),
              float (*next_frame_callback) (void* arg),
              void* callback_arg)
{
   jc_t* jc = malloc(sizeof(jc_t));
   jc->note_on = note_on_callback;
   jc->note_off = note_off_callback;
   jc->sustain = sustain_callback;
   jc->next_frame = next_frame_callback;
   jc->callback_arg = callback_arg;
  
   jc->client = jack_client_open(client_name, JackNullOption, NULL);

   if(jc->client == NULL) {
      jc_errno = JC_SUCCESS;
      free(jc);
      return NULL;
   }

   jc->sample_rate = jack_get_sample_rate(jc->client);
   jc->midiin_port = jack_port_register(jc->client,
                                        "midi_in",
                                        JACK_DEFAULT_MIDI_TYPE,
                                        JackPortIsInput,
                                        0);
   if(jc->midiin_port == NULL) {
      jc_errno = JC_REGMIDI;
      free(jc);
      return NULL;
   }

   jc->output_port = jack_port_register(jc->client,
                                        "out",
                                        JACK_DEFAULT_AUDIO_TYPE,
                                        JackPortIsOutput,
                                        0);
   if(jc->output_port == NULL) {
      jc_errno = JC_REGAUDIO;
      free(jc);
      return NULL;
   }

   jack_set_process_callback(jc->client, process, jc);

   return jc;
}


void jc_activate(jc_t* jc)
{
   if(jack_activate(jc->client) != 0) {
      jc_errno = JC_ACTIVATE;
   }
   else {
      jc_errno = JC_SUCCESS;
   }
}

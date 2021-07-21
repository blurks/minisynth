#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <jack/jack.h>
#include <jack/midiport.h>

#include "synth.h"

jack_client_t* client;
jack_nframes_t sample_rate;
jack_port_t* output_port;
jack_port_t* midiin_port;

int process(jack_nframes_t nframes, void* args);

int main(int argc __attribute__((unused)), char** argv __attribute__((unused))) {
  /* Configure jack client. */
  client = jack_client_open("sinesynth", JackNullOption, NULL);
  if(client == NULL) {
    fprintf(stderr, "Couldn't open client.\n");
    exit(1);
  }

  sample_rate = jack_get_sample_rate(client);
  midiin_port = jack_port_register(client,
				   "midi_in",
				   JACK_DEFAULT_MIDI_TYPE,
				   JackPortIsInput,
				   0);
  if(midiin_port == NULL) {
    fprintf(stderr, "Couldn't register midi input port.\n");
    exit(1);
  }

  output_port = jack_port_register(client,
				   "out",
				   JACK_DEFAULT_AUDIO_TYPE,
				   JackPortIsOutput,
				   0);
  if(output_port == NULL) {
    fprintf(stderr, "Couldn't register audio output port.\n");
    exit(1);
  }

  jack_set_process_callback(client, process, 0);

  /* Configure Synth */
  init_synth((long)sample_rate);
  
  /* Connect to Jack Server. */
  if(jack_activate(client) != 0) {
    fprintf(stderr, "Couldn't activate client.\n");
    exit(1);
  }
  while(1) {
    sleep(1);
  }
  return 0;

}

int process(jack_nframes_t nframes, void* args __attribute__((unused))) {
  jack_default_audio_sample_t* output_port_buffer =
    (jack_default_audio_sample_t*) jack_port_get_buffer(output_port, nframes);
  void* midiin_port_buffer = jack_port_get_buffer(midiin_port, nframes);
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
	note_on(midiin_event.buffer[1], midiin_event.buffer[2]);
      }
      else if((midiin_event.buffer[0] & 0xf0) == 0x80 ||
	      ((midiin_event.buffer[0] & 0xf0) == 0x90 &&
	       midiin_event.buffer[2] == 0x00 )) {
	note_off(midiin_event.buffer[1]);
      }
      event_index++;
      jack_midi_event_get(&midiin_event, midiin_port_buffer, event_index);
    }
    output_port_buffer[i] = next_frame();
  }
  return 0;
}

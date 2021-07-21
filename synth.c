#include <math.h>
#include <stdlib.h>

#include "synth.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

long framerate = 0;

struct note note_array[128];

float env_attack = DEFAULT_ENV_ATTACK;
float env_hold = DEFAULT_ENV_HOLD;
float env_decay = DEFAULT_ENV_DECAY;
float env_release = DEFAULT_ENV_RELEASE;

float harmonics[5] = {0.5f, 0.1f, 0.01f, 0.05f, 0.005f};

void init_synth(long f) {
  framerate = f;
  for(size_t i=0; i<128; i++) {
    note_array[i].freq = 440.0f * powf(2.0f, ((float)i - 69.0f) / 12.0f);
    note_array[i].state = NOTE_OFF;
    note_array[i].on_since = 0;
    note_array[i].last_event = 0;
    note_array[i].amplitude = 0.0f;
    note_array[i].velocity = 0.0f;
  }
}

void note_on(unsigned char note_index, unsigned char velocity) {
  struct note* n = &(note_array[(size_t) note_index]);
  if(n->state == NOTE_OFF) {
    n->on_since = 0;
  }
  n->state = NOTE_ATTACK;
  float normalized_velocity = (float)velocity / 127.0f;
  n->velocity = n->amplitude < normalized_velocity ?
    normalized_velocity : n->amplitude;
  n->last_event = n->on_since;
}

void note_off(unsigned char note_index) {
  struct note* n = &(note_array[(size_t) note_index]);
  if(n->state != NOTE_OFF) {
    n->state = NOTE_RELEASE;
    n->last_event = n->on_since;
  }
}

float note_envelope(struct note* n) {
  /* amp_inc - amplitude increment per frame
   env_val - value of envelope phase like env_attack, env_decay, ...

   amp_inc = 1000.0 / (env_val * framerate)
  */
  if(n->state == NOTE_ATTACK) {
    n->amplitude += 1000.0f / (env_attack * framerate);
    if(n->amplitude >= 1.0f) {
      n->state = NOTE_HOLD;
      n->amplitude = 1.0f;
      n->last_event = n->on_since;
    }
  }
  else if(n->state == NOTE_HOLD) {
    if((n->on_since - n->last_event) > ((env_hold / 1000.0f) * framerate)) {
      n->state = NOTE_DECAY;
      n->last_event = n->on_since;
    }
  }
  else if(n->state == NOTE_DECAY) {
    n->amplitude -= 1000.0f / (env_decay * framerate);
    if(n->amplitude <= 0.0f) {
      n->state = NOTE_OFF;
      n->amplitude = 0.0f;
      n->last_event = n->on_since;
    }
  }
  else if(n->state == NOTE_RELEASE) {
    n->amplitude -= 1000.0f / (env_release * framerate);
    if(n->amplitude <= 0.0f) {
      n->state = NOTE_OFF;
      n->amplitude = 0.0f;
      n->last_event = n->on_since;
    }
  }
  else {
    return 0.0f;
  }
  return n->amplitude;
}

inline float sin_freq(float freq, long frame) {
  float period_frame_length = (float)framerate / freq;
  //  float mod_frame = (float)(frame % (long) period_frame_length);
  //  return sinf(mod_frame * 2 * M_PI / period_frame_length);
  return sinf((float)frame * 2 * M_PI / period_frame_length);
}

float note_waveform(struct note* n) {
  float normalizer = 1.0f + harmonics[0] +harmonics[1]+ harmonics[2] +
    harmonics[3] + harmonics[4];
  float frame = sin_freq(n->freq, n->on_since);
  for(size_t i=0; i<5; i++) {
    frame += harmonics[i] * sin_freq(n->freq * (float)(i + 2), n->on_since);
  }
  return frame / normalizer;
}

float note_nextframe(struct note* n) {
  n->on_since += 1;
  return n->velocity * note_envelope(n) * note_waveform(n);
}

float next_frame() {
  float frame = 0.0f;
  for(size_t i=0; i<128; i++) {
    if(note_array[i].state != NOTE_OFF) {
      frame += 0.2f * note_nextframe(&(note_array[i]));
    }
  }
  return frame;
}

#include <math.h>
#include <stdlib.h>

#include "synth.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct synth* init_synth(long framerate,
                         float env_attac,
                         float env_hold,
                         float env_decay,
                         float env_release,
                         float h1, float h2, float h3, float h4, float h5) {
   struct synth* new_synth = malloc(sizeof(struct synth));
   if(new_synth) {
      new_synth->framerate = framerate;
      new_synth->env_attac = env_attac;
      new_synth->env_hold = env_hold;
      new_synth->env_decay = env_decay;
      new_synth->env_release = env_release;

      new_synth->harmonics[0] = h1;
      new_synth->harmonics[1] = h2;
      new_synth->harmonics[2] = h3;
      new_synth->harmonics[3] = h4;
      new_synth->harmonics[4] = h5;
    
      for(size_t i=0; i<128; i++) {
         new_synth->note_array[i].freq =
            440.0f * powf(2.0f, ((float)i - 69.0f) / 12.0f);
         new_synth->note_array[i].state = NOTE_OFF;
         new_synth->note_array[i].on_since = 0;
         new_synth->note_array[i].last_event = 0;
         new_synth->note_array[i].amplitude = 0.0f;
         new_synth->note_array[i].velocity = 0.0f;
      }
   }
   return new_synth;
}

void note_on(struct synth* s, unsigned char note_index,
             unsigned char velocity) {
   struct note* n = &(s->note_array[(size_t) note_index]);
   if(n->state == NOTE_OFF) {
      n->on_since = 0;
   }
   n->state = NOTE_ATTACK;
   float normalized_velocity = (float)velocity / 127.0f;
   n->velocity = n->amplitude < normalized_velocity ?
      normalized_velocity : n->amplitude;
   n->last_event = n->on_since;
}

void note_off(struct synth* s, unsigned char note_index) {
   struct note* n = &(s->note_array[(size_t) note_index]);
   if(n->state != NOTE_OFF && !s->sustain) {
      n->state = NOTE_RELEASE;
      n->last_event = n->on_since;
   }
   else if(n->state != NOTE_OFF && s-> sustain) {
      n->state = NOTE_PEDAL;
   }
}

void sustain_pedal(struct synth* s, unsigned char value) {
   if(value == 0x7f) {
      s->sustain = 1;
   }
   else {
      s->sustain = 0;
      // This could probably be integrated in note_envelope
      for(unsigned char i=0; i<128; i++) {
         struct note* n = &(s->note_array[(size_t) i]);
         if(n->state == NOTE_PEDAL) {
            note_off(s, i);
         }
      }
   }
}

float note_envelope(struct synth* s, struct note* n) {
   /* amp_inc - amplitude increment per frame
      env_val - value of envelope phase like env_attack, env_decay, ...

      amp_inc = 1000.0 / (env_val * framerate)
   */
   if(n->state == NOTE_ATTACK) {
      n->amplitude += n->velocity * 1000.0f / (s->env_attac * s->framerate);
      if(n->amplitude >= n->velocity) {
         n->state = NOTE_HOLD;
         n->amplitude = n->velocity;
         n->last_event = n->on_since;
      }
   }
   else if(n->state == NOTE_HOLD) {
      if((n->on_since - n->last_event) > ((s->env_hold / 1000.0f) * s->framerate)) {
         n->state = NOTE_DECAY;
         n->last_event = n->on_since;
      }
   }
   else if(n->state == NOTE_DECAY || n->state == NOTE_PEDAL) {
      n->amplitude -= n->velocity * 1000.0f / (s->env_decay * s->framerate);
      if(n->amplitude <= 0.0f) {
         n->state = NOTE_OFF;
         n->amplitude = 0.0f;
         n->last_event = n->on_since;
      }
   }
   else if(n->state == NOTE_RELEASE) {
      n->amplitude -= n->velocity * 1000.0f / (s->env_release * s->framerate);
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

inline float sin_freq(struct synth* s, float freq, long frame) {
   float period_frame_length = (float)s->framerate / freq;
   //  float mod_frame = (float)(frame % (long) period_frame_length);
   //  return sinf(mod_frame * 2 * M_PI / period_frame_length);
   return sinf((float)frame * 2 * M_PI / period_frame_length);
}

float note_waveform(struct synth* s, struct note* n) {
   float normalizer = 1.0f + fabsf(s->harmonics[0]) + fabsf(s->harmonics[1]) +
      fabsf(s->harmonics[2]) + fabsf(s->harmonics[3]) + fabsf(s->harmonics[4]);
   float frame = sin_freq(s, n->freq, n->on_since);
   for(size_t i=0; i<5; i++) {
      frame += s->harmonics[i] * sin_freq(s, n->freq * (float)(i + 2), n->on_since);
   }
   return frame / normalizer;
}

float* waveform(struct synth* s, long num) {
   struct note n = {
      .freq = s->framerate / num,
      .on_since = 0,
      .last_event = 0,
      .velocity = 1.0f,
      .amplitude = 1.0f
   };
   float* frames = malloc(num * sizeof(float));
   for(long i = 0; i < num; i++) {
      frames[i] = note_waveform(s, &n);
      n.on_since++;
   }
   return frames;
}

float note_nextframe(struct synth* s, struct note* n) {
   n->on_since += 1;
   return note_envelope(s, n) * note_waveform(s, n);
}

float next_frame(struct synth* s) {
   float frame = 0.0f;
   for(size_t i=0; i<128; i++) {
      if(s->note_array[i].state != NOTE_OFF) {
         frame += 0.2f * note_nextframe(s, &(s->note_array[i]));
      }
   }
   return frame;
}

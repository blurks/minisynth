#ifndef SYNTH_H
#define SYNTH_H 1

enum note_state {
  NOTE_OFF,
  NOTE_ATTACK,
  NOTE_HOLD,
  NOTE_DECAY,
  NOTE_RELEASE
};

struct note {
  float freq;
  long on_since;
  long last_event;
  float velocity;
  float amplitude;
  enum note_state state;
};

struct synth {
  long framerate;
  struct note note_array[128];
  float env_attac;
  float env_hold;
  float env_decay;
  float env_release;
  float harmonics[5];
};

#define DEFAULT_ENV_ATTACK 50.0f
#define DEFAULT_ENV_HOLD 100.0f
#define DEFAULT_ENV_DECAY 4000.0f
#define DEFAULT_ENV_RELEASE 200.0f

struct synth* init_synth(long framerate, float env_attac, float env_hold,
                         float env_decay, float env_release, float h1, float h2,
                         float h3, float h4, float h5);
void note_on(struct synth* s, unsigned char note_index,
             unsigned char velocity);
void note_off(struct synth* s, unsigned char note_index);
float next_frame(struct synth* s);

#endif

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

#define DEFAULT_ENV_ATTACK 50.0f
#define DEFAULT_ENV_HOLD 100.0f
#define DEFAULT_ENV_DECAY 4000.0f
#define DEFAULT_ENV_RELEASE 200.0f

extern float env_attack;
extern float env_hold;
extern float env_decay;
extern float env_release;

extern float harmonics[5];

void init_synth(long f);
void note_on(unsigned char note_index, unsigned char velocity);
void note_off(unsigned char note_index);
float next_frame();

#endif

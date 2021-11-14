#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jackclient.h"
#include "synth.h"

#define PROG_NAME "minisynth"

float env_attac = DEFAULT_ENV_ATTACK;
float env_hold = DEFAULT_ENV_HOLD;
float env_decay = DEFAULT_ENV_DECAY;
float env_release = DEFAULT_ENV_RELEASE;

float h1 = 1.0;
float h2 = 0.333;
float h3 = 0.111;
float h4 = 0.006;
float h5 = 0.034;

struct synth* synthesizer;
jc_t* jackclient;

int main(int argc __attribute__((unused)), char** argv __attribute__((unused)))
{
    synthesizer = init_synth((long)44800,
                             env_attac,
                             env_hold,
                             env_decay,
                             env_release,
                             h1, h2, h3, h4, h5);
    
    jackclient = jc_init(PROG_NAME,
                         &note_on,
                         &note_off,
                         &next_frame,
                         synthesizer);

    synthesizer->framerate = (long) jackclient->sample_rate;

    jc_activate(jackclient);
                         
  while(1) {
    sleep(1);
  }
  return 0;
}

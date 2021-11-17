#define _POSIX_C_SOURCE 2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jackclient.h"
#include "synth.h"

#ifndef PROGNAME
#define PROGNAME "minisynth"
#endif

float env_attack = DEFAULT_ENV_ATTACK;
float env_hold = DEFAULT_ENV_HOLD;
float env_decay = DEFAULT_ENV_DECAY;
float env_release = DEFAULT_ENV_RELEASE;

float h1 = 1.0;
float h2 = 0.333;
float h3 = 0.111;
float h4 = 0.037;
float h5 = 0.012;

struct synth* synthesizer;
jc_t* jackclient;


inline void usage(FILE* s)
{
  fprintf(s,
          "Usage: %s [OPTIONS]\n\n"
          "A simple additive synthesizer with 5 harmonics.\n"
          "Options:\n"
          "\t-1 ... 5 <f>\tset harmonic amplitudes\n"
          "\t-a <f>\tset attack in ms\n"
          "\t-o <f>\tset hold in ms\n"
          "\t-d <f>\tset decay in ms\n"
          "\t-r <f>\tset release in ms\n"
          "\t-h\t show this help message\n",
          PROGNAME);
}


int main(int argc, char** argv)
{
  char* optstr = "h1:2:3:4:5:a:o:d:r:";
  char opt;
  float* arg;
  while((opt = getopt(argc, argv, optstr)) > 0) {
    arg = NULL;
    switch(opt) {
    case 'h':
      usage(stdout);
      exit(0);
      break;
    case '1':
      arg = &h1;
      break;
    case '2':
      arg = &h2;
      break;
    case '3':
      arg = &h3;
      break;
    case '4':
      arg = &h4;
      break;
    case '5':
      arg = &h5;
      break;
    case 'a':
      arg = &env_attack;
      break;
    case 'o':
      arg = &env_hold;
      break;
    case 'd':
      arg = &env_decay;
      break;
    case 'r':
      arg = &env_release;
      break;
    default:
      usage(stderr);
      exit(1);
    }
    if(arg && optarg) {
      char* endptr;
      *arg = strtof(optarg, &endptr);
      if(*optarg != '\0' && *endptr != '\0') {
        fprintf(stderr, "Invalid argument: %s. Must be a float!\n", optarg);
        usage(stderr);
        exit(1);
      }
    }
  }
  
  synthesizer = init_synth((long)44800,
                           env_attack,
                           env_hold,
                           env_decay,
                           env_release,
                           h1, h2, h3, h4, h5);
    
  jackclient = jc_init(PROGNAME,
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

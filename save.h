#ifndef SAVE_H
#define SAVE_H 1

#include "synth.h"

int save_settings(const struct synth* s, const char* filename);
int load_settings(struct synth* s, const char* filename);
#endif

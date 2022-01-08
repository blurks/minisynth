#include <stdio.h>

#include "save.h"
#include "synth.h"

int save_settings(const struct synth* s, const char* filename) {
   FILE* fd = fopen(filename, "w");
   if(fd == NULL) {
      // perror(filename);
      return -1;
   }
   int bytes = fprintf(fd, "attack %f\n", s->env_attac);
   bytes += fprintf(fd, "hold %f\n", s->env_hold);
   bytes += fprintf(fd, "decay %f\n", s->env_decay);
   bytes += fprintf(fd, "release %f\n", s->env_release);
   bytes += fprintf(fd, "h1 %f\n", s->harmonics[0]);
   bytes += fprintf(fd, "h2 %f\n", s->harmonics[1]);
   bytes += fprintf(fd, "h3 %f\n", s->harmonics[2]);
   bytes += fprintf(fd, "h4 %f\n", s->harmonics[3]);
   bytes += fprintf(fd, "h5 %f\n", s->harmonics[4]);
   fclose(fd);
   return bytes;
}



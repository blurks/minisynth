/* TODO:
 * - get rid of global variables (might be impossible)
 * - envelope controls
 */
#include <assert.h>
#include <forms.h>
#include <stdio.h>
#include <stdlib.h>

#include "synth.h"
#include "jackclient.h"


FL_OBJECT* plot;
struct synth* synthesizer;

void draw_plot()
{
   long num = 200;
   float* x = malloc(num * sizeof(float));
   for(long i = 0; i < num; i++) {
      x[i] = (float)i / (float)num;
   }
   float* y = waveform(synthesizer, num);

   fl_set_xyplot_data(plot, x, y, num, NULL, NULL, NULL);
   free(x);
   free(y);     
}

void harmonic_slider_cb(FL_OBJECT* slider, long harmonic)
{
   synthesizer->harmonics[harmonic] = fl_get_slider_value(slider);
   draw_plot(synthesizer);
}

void build_slider(int harmonic, int x, int y, int w, int h)
{
   char label[16];
   snprintf(label, 16, "h%d", harmonic + 1);
   FL_OBJECT* obj = fl_add_valslider(FL_VERT_NICE_SLIDER, x, y, w, h, label);
   fl_set_slider_bounds(obj, 1.0, -1.0);
   fl_set_slider_value(obj, synthesizer->harmonics[harmonic]);
   fl_set_object_callback(obj, harmonic_slider_cb, harmonic);
}

void build_harmonic_sliders(int x, int y, int w, int h)
{
   int slider_num = 5;
   int slider_width = w / slider_num;
   for(int i = 0; i < slider_num; i++) {
      build_slider(i, x + (slider_width * i), y, slider_width - 1, h-20);
   }
}

void env_dial_cb(FL_OBJECT* dial, long index)
{
   float val = fl_get_dial_value(dial);
   switch(index) {
   case 0:
      synthesizer->env_attac = val;
      break;
   case 1:
      synthesizer->env_hold = val;
      break;
   case 2:
      synthesizer->env_decay = val;
      break;
   case 3:
      synthesizer->env_release = val;
      break;
   default:
      break;
   }
}

void build_env_dial(int index, int x, int y, int w, int h)
{
   assert(index < 4);
   char* label;
   float val;
   switch(index) {
   case 0:
      label = "Attack";
      val = DEFAULT_ENV_ATTACK;
      break;
   case 1:
      label = "Hold";
      val = DEFAULT_ENV_HOLD;
      break;
   case 2:
      label = "Decay";
      val = DEFAULT_ENV_DECAY;
      break;
   case 3:
      label = "Release";
      val = DEFAULT_ENV_RELEASE;
      break;
   default:
      label = "";
      val = 0.0;
      break;
   }

   FL_OBJECT* obj = fl_add_dial(FL_FILL_DIAL, x, y, w, h, label);
   fl_set_object_callback(obj, env_dial_cb, index);
   fl_set_dial_bounds(obj, 0.0, 2 * val);
   fl_set_dial_angles(obj, 45, 315);
   fl_set_dial_value(obj, val);
}

void build_env_dials(int x, int y, int w, int h)
{
   for(int i=0; i<4; i++) {
      build_env_dial(i, x, y + (i * h / 4), w, h / 4 - 20);
   }
}

FL_FORM* build_gui()
{
   int width = 400;
   int height = 400;
   FL_FORM* form = fl_bgn_form(FL_FLAT_BOX, width, height);

   build_harmonic_sliders(0, height / 2, width - 100, height / 2);
   
   plot = fl_add_xyplot(FL_NORMAL_XYPLOT, 0, 0, width - 100, height / 2, NULL);
   fl_set_xyplot_ygrid(plot, FL_GRID_MINOR);
   fl_set_xyplot_xbounds(plot, 0.0, 1.0);
   fl_set_xyplot_ybounds(plot, -1.0, 1.0);
   fl_set_xyplot_xtics(plot, 1, 0);
   fl_set_xyplot_ytics(plot, 2, 2);
   draw_plot(synthesizer);

   build_env_dials(300, 0, 100, height);
   
   fl_end_form();
   
   return form;
}

int main(int argc, char** argv)
{
   synthesizer = init_synth(48000,
                            DEFAULT_ENV_ATTACK,
                            DEFAULT_ENV_HOLD,
                            DEFAULT_ENV_DECAY,
                            DEFAULT_ENV_RELEASE,
                            0.5, 0.0, 0.0, 0.0 , 0.0);

   jc_t* jackclient = jc_init(
      argv[0],
      (void (*) (void*, unsigned char, unsigned char)) &note_on,
      (void (*) (void*, unsigned char)) &note_off,
      (void (*) (void*, unsigned char)) &sustain_pedal,
      (float (*) (void*)) &next_frame,
      synthesizer
      );

   synthesizer->framerate = (long) jackclient->sample_rate;

   jc_activate(jackclient);
   
   fl_initialize(&argc, argv, 0, 0, 0);
   FL_FORM* form = build_gui();
   fl_show_form(form, FL_PLACE_FREE, FL_FULLBORDER, "minisynth");
   fl_do_forms();

   fl_hide_form(form);
   fl_finish();
   return 0;
}

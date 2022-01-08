/* TODO:
 * - get rid of global variables (might be impossible)
 */
#include <assert.h>
#include <forms.h>
#include <stdio.h>
#include <stdlib.h>

#include "jackclient.h"
#include "save.h"
#include "synth.h"


FL_OBJECT* plot;
struct synth* synthesizer;

/* Drawing the gui */
void build_env_dial(int index, int x, int y, int w, int h);
void build_env_dials(int x, int y, int w, int h);
FL_FORM* build_gui();
void build_harmonic_sliders(int x, int y, int w, int h);
void build_harmonic_slider(int harmonic, int x, int y, int w, int h);
void build_toolbar(int x, int y, int w, int h);
/* Object callbacks */
void env_dial_cb(FL_OBJECT* dial, long index);
void harmonic_input_cb(FL_OBJECT* input, long harmonic);
void harmonic_slider_cb(FL_OBJECT* slider, long harmonic);
void save_btn_cb(FL_OBJECT* btn, long data);
void load_btn_cb(FL_OBJECT* btn, long data);
/* Other functions */
void draw_plot();
void update_harmonics(long harmonic, float val);


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

/* Draw the gui */

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
   int toolbar_height = 40;
   int height = 400;
   FL_FORM* form = fl_bgn_form(FL_FLAT_BOX, width, height  + toolbar_height);

   build_toolbar(0, 0, width, toolbar_height);
   
   plot = fl_add_xyplot(FL_NORMAL_XYPLOT,
                        0,
                        toolbar_height,
                        width - 100,
                        height / 2,
                        NULL);
   fl_set_xyplot_ygrid(plot, FL_GRID_MINOR);
   fl_set_xyplot_xbounds(plot, 0.0, 1.0);
   fl_set_xyplot_ybounds(plot, -1.0, 1.0);
   fl_set_xyplot_xtics(plot, 1, 0);
   fl_set_xyplot_ytics(plot, 2, 2);
   draw_plot(synthesizer);
   
   build_harmonic_sliders(0,
                          toolbar_height + height / 2,
                          width - 100,
                          height / 2);
   
   build_env_dials(300, toolbar_height, 100, height);
   
   fl_end_form();
   
   return form;
}


void build_harmonic_sliders(int x, int y, int w, int h)
{
   int slider_num = 5;
   int slider_width = w / slider_num;
   for(int i = 0; i < slider_num; i++) {
      build_harmonic_slider(i,
                            x + (slider_width * i),
                            y,
                            slider_width - 1,
                            h - 20);
   }
}


void build_harmonic_slider(int harmonic, int x, int y, int w, int h)
{
   char label[16];
   snprintf(label, 16, "h%d", harmonic + 1);
   FL_OBJECT* slider = fl_add_slider(FL_VERT_NICE_SLIDER,
                                  x , y + 20, w , h - 20, label);
   fl_set_slider_bounds(slider, 1.0, -1.0);
   fl_set_slider_value(slider, synthesizer->harmonics[harmonic]);
   fl_set_object_callback(slider, harmonic_slider_cb, harmonic);

   FL_OBJECT* input = fl_add_spinner(FL_FLOAT_SPINNER, x, y, w, 20, NULL);
   fl_set_spinner_value(input, synthesizer->harmonics[harmonic]);
   fl_set_spinner_bounds(input, -1.0, 1.0);
   fl_set_spinner_precision(input, 3);
   fl_set_spinner_step(input, 0.001);
   fl_set_object_callback(input, harmonic_input_cb, harmonic);

   slider->u_vdata = input;
   input->u_vdata = slider;
}


void build_toolbar(int x, int y, int w, int h)
{
   FL_OBJECT* save_btn = fl_add_button(FL_BUTTON, x, y, w / 2, h, "Save");
   FL_OBJECT* load_btn = fl_add_button(FL_BUTTON, x + w / 2, y, w / 2, h,
                                       "Load");
   fl_set_object_callback(save_btn, save_btn_cb, 0);
   fl_set_object_callback(load_btn, load_btn_cb, 0);
}

/* Object callbacks */

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


void harmonic_slider_cb(FL_OBJECT* slider, long harmonic)
{
   float val = fl_get_slider_value(slider);
   fl_set_spinner_value(slider->u_vdata, val);
   update_harmonics(harmonic, val);
}


void harmonic_input_cb(FL_OBJECT* input, long harmonic)
{
   float val = (float) fl_get_spinner_value(input);
   fl_set_slider_value(input->u_vdata, val);
   update_harmonics(harmonic, val);
}


void save_btn_cb(__attribute__((unused)) FL_OBJECT* btn,
                 __attribute__((unused)) long data)
{
   char* dir = getenv("HOME");
   if(!dir) {
      dir = "/home/";
   }
   const char* filename = fl_show_fselector("Save settings", dir, "*",
                                            "minisynth.preset");
   if(save_settings(synthesizer, filename) < 0) {
      perror(filename);
   }
}


void load_btn_cb(__attribute__((unused)) FL_OBJECT* btn,
                 __attribute__((unused)) long data)
{
   fl_show_fselector("Load settings", "/home/", "*", NULL);
}

/* Other functions */

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


void update_harmonics(long harmonic, float val)
{
   synthesizer->harmonics[harmonic] = val;
   draw_plot(synthesizer);
}

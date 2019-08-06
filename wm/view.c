#include "view.h"
#include "xapi.h"
#include <limits.h>

Bool debugpicks = False;
 
void mat4mul(float *mat4, float *vec4, float *outvec4) {
  for (int i = 0; i < 4; i++) {
   float res = 0.0;
    for (int j = 0; j < 4; j++) {
      res += mat4[i*4 + j] * vec4[j];
    }
    outvec4[i] = res;
  }
  /*
  printf("|%f,%f,%f,%f||%f|   |%f|\n"
         "|%f,%f,%f,%f||%f|   |%f|\n"
         "|%f,%f,%f,%f||%f| = |%f|\n"
         "|%f,%f,%f,%f||%f|   |%f|\n",
         mat4[0], mat4[1], mat4[2], mat4[3],  vec4[0], outvec4[0],
         mat4[4], mat4[5], mat4[6], mat4[7],  vec4[1], outvec4[1],
         mat4[8], mat4[9], mat4[10],mat4[11], vec4[2], outvec4[2],
         mat4[12],mat4[13],mat4[14],mat4[15], vec4[3], outvec4[3]);
  */
}

void view_to_space(View *view, float screenx, float screeny, float *spacex, float *spacey) {
  float screen2space[4*4] = {view->screen[2]/view->width,0,0,view->screen[0],
                             0,-view->screen[3]/view->height,0,view->screen[1],
                             0,0,1,0,
                             0,0,0,1};
  float space[4] = {screenx, screeny, 0., 1.};
  float outvec[4];
  mat4mul(screen2space, space, outvec);
  *spacex = outvec[0];
  *spacey = outvec[1];
}
void view_from_space(View *view, float spacex, float spacey, float *screenx, float *screeny) {
  float space2screen[4*4] = {view->width/view->screen[2], 0., 0., -view->width*view->screen[0]/view->screen[2],
                             0., -view->height/view->screen[3], 0., -view->height*view->screen[1]/view->screen[3],
                             0., 0., 1., 0.,
                             0., 0., 0., 1.};
  float space[4] = {spacex, spacey, 0., 1.};
  float outvec[4];
  mat4mul(space2screen, space, outvec);
  *screenx = outvec[0];
  *screeny = outvec[1];
}




void view_abstract_draw(View *view, Item **items, ItemFilter *filter) {
  for (; items && *items; items++) {
    if (!filter || filter(*items)) {
      (*items)->type->draw(view, *items);
    }
  }
}

void view_draw(GLint fb, View *view, Item **items, ItemFilter *filter) {
  gl_check_error("draw0");
  glBindFramebuffer(GL_FRAMEBUFFER, fb);
  glEnablei(GL_BLEND, 0);
  glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
  gl_check_error("draw1");
  view->picking = 0;
  view_abstract_draw(view, items, filter);
  gl_check_error("draw2");
}

void view_draw_picking(GLint fb, View *view, Item **items, ItemFilter *filter) {
  gl_check_error("view_draw_picking1");
  glBindFramebuffer(GL_FRAMEBUFFER, fb);
  view->picking = 1;
  view_abstract_draw(view, items, filter);
  gl_check_error("view_draw_picking2");
}
  
void view_pick(GLint fb, View *view, int x, int y, int *winx, int *winy, Item **returnitem) {
  float data[4];
  memset(data, 0, sizeof(data));
  glReadPixels(x, view->height - y, 1, 1, GL_RGBA, GL_FLOAT, (GLvoid *) data);
  gl_check_error("pick2");
  if (debugpicks) {
    fprintf(stderr, "Pick %d,%d -> %f,%f,%f,%f\n", x, y, data[0], data[1], data[2], data[3]);
  }
  *winx = 0;
  *winy = 0;
  *returnitem = NULL;
  if (data[2] != 0.0) {
    *returnitem = item_get(data[2] * (float) INT_MAX);
    if (*returnitem) {
      *winx = (int) (data[0] * (*returnitem)->width);
      *winy = (int) (data[1] * (*returnitem)->height);
    }
  }
  if (debugpicks) {
    if (*returnitem) {
      fprintf(stderr, "  -> %d,%d,%d\n", (*returnitem)->id, *winx, *winy);
    } else {
      fprintf(stderr, "  -> NULL\n");
    }
 }
}

View *view_load(Atom name) {
  View *view = malloc(sizeof(View));
  view->name = name;
  char *strname = XGetAtomName(display, name);

  {
    char str_attr_layer[strlen(strname) + 7];
    strcpy(str_attr_layer, strname);
    strcpy(str_attr_layer + strlen(strname), "_LAYER");
  
    view->attr_layer = XInternAtom(display, str_attr_layer, False);
  }
  {
    char str_attr_view[strlen(strname) + 6];
    strcpy(str_attr_view, strname);
    strcpy(str_attr_view + strlen(strname), "_VIEW");
    view->attr_view = XInternAtom(display, str_attr_view, False);
  }
  
  XFree(strname);
 
  Atom type_return;
  int format_return;
  unsigned long nitems_return;
  unsigned long bytes_after_return;
  unsigned char *prop_return;

  XGetWindowProperty(display, root, view->attr_layer, 0, sizeof(Atom), 0, AnyPropertyType,
                     &type_return, &format_return, &nitems_return, &bytes_after_return, &prop_return);
  if (type_return != None) {
   view->layer = *(Atom *) prop_return;
  }
  XFree(prop_return);

  XGetWindowProperty(display, root, view->attr_view, 0, sizeof(Atom), 0, AnyPropertyType,
                     &type_return, &format_return, &nitems_return, &bytes_after_return, &prop_return);
  if (type_return != None) {
    memcpy((void *) view->screen, prop_return, sizeof(float) * 4);
  }
  XFree(prop_return);
}

View **view_load_all(void) {
  Atom type_return;
  int format_return;
  unsigned long nitems_return;
  unsigned long bytes_after_return;
  unsigned char *prop_return;

  XGetWindowProperty(display, root, IG_VIEWS, 0, 100000, 0, AnyPropertyType,
                     &type_return, &format_return, &nitems_return, &bytes_after_return, &prop_return);
  if (type_return == None) {
    XFree(prop_return);
    return NULL;
  }
  
  View **res = malloc(sizeof(View *) * (nitems_return + 1));
  
  for (int i=0; i < nitems_return; i++) {
    res[i] = view_load(((Atom *) prop_return)[i]);
  }
  res[nitems_return] = NULL;
  XFree(prop_return);

  return res;
}

void view_set_screen(View *view, float screen[4]) {
  for (int i = 0; i < 4; i++) {
    view->screen[i] = screen[i];
  }
  XChangeProperty(display, root, view->attr_view, XA_FLOAT, 32, PropModeReplace, screen, 4);
}

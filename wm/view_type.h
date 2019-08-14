#ifndef VIEW_TYPE
#define VIEW_TYPE

#include "xapi.h"

typedef struct {
  Atom name;
  Atom layer;

  Atom attr_layer;
  Atom attr_view;
  Atom attr_size;

  float screen[4];
  int width;
  int height;
  int picking;
} View;

#endif

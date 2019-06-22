#ifndef VIEW
#define VIEW

#include "item.h"
#include "view_type.h"

extern void mat4mul(float *mat4, float *vec4, float *outvec4);
extern void view_to_space(View *view, float screenx, float screeny, float *spacex, float *spacey);
extern void view_from_space(View *view, float spacex, float spacey, float *screenx, float *screeny);

extern void view_abstract_draw(View *view, Item **items);
extern void view_draw(GLint fb, View *view, Item **items);
extern void view_pick(GLint fb, View *view, Item **items, int x, int y, int *winx, int *winy, Item **returnitem);

#endif

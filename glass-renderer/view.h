#ifndef VIEW
#define VIEW

#include "item.h"
#include "view_type.h"

typedef Bool ItemFilter(Item *item);

extern void mat4mul(float *mat4, float *vec4, float *outvec4);
extern void view_to_space(View *view, float screenx, float screeny, float *spacex, float *spacey);
extern void view_from_space(View *view, float spacex, float spacey, float *screenx, float *screeny);

extern void view_abstract_draw(View *view, Item **items, ItemFilter *filter);
extern void view_draw(GLint fb, View *view, Item **items, ItemFilter *filter);
extern void view_draw_picking(GLint fb, View *view, Item **items, ItemFilter *filter);
extern void view_pick(GLint fb, View *view, int x, int y, int *winx, int *winy, Item **returnitem);

extern void view_load_layer(View *view);
extern void view_load_screen(View *view);
extern View *view_load(Atom name);
extern View **view_load_all(void);
extern void view_free(View *view);
extern void view_free_all(View **views);
extern void view_update(View *view);
extern View *view_find(View **views, Atom name);

#endif

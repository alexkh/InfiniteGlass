#include "mainloop.h"
#include "list.h"
#include "debug.h"
#include <string.h>

List *mainloop_event_handlers = NULL;
List *timeout_handlers = NULL;

Bool event_match(XEvent *event, XEvent *match_event, XEvent *match_mask) {
 char *e = (char *) event;
 char *me = (char *) match_event;
 char *mm = (char *) match_mask;

  for (size_t idx = 0; idx < sizeof(XEvent); idx++) {
    if ((e[idx] & mm[idx]) != (me[idx] & mm[idx])) {
      return False;
    }
  }
  return True;
}

void mainloop_install_event_handler(EventHandler *handler) {
  if (!mainloop_event_handlers) mainloop_event_handlers = list_create();
  list_append(mainloop_event_handlers, (void *) handler);
  // Fetch existing mask and OR!!!
  if (handler->match_mask.xany.window) {
    XSelectInput(display, handler->match_event.xany.window, handler->event_mask);
  }
}

void mainloop_uninstall_event_handler(EventHandler *handler) {
  list_remove(mainloop_event_handlers, (void *) handler);
}


void mainloop_install_timeout_handler(TimeoutHandler *handler) {
  if (!timeout_handlers) timeout_handlers = list_create();
  list_append(timeout_handlers, (void *) handler);
}

void mainloop_uninstall_timeout_handler(TimeoutHandler *handler) {
  list_remove(timeout_handlers, (void *) handler);
}



Bool mainloop_event_handle(XEvent *event) {
  if (!mainloop_event_handlers) return False;
  for (int idx = 0; idx < mainloop_event_handlers->count; idx++) {
    EventHandler *handler = (EventHandler *) mainloop_event_handlers->entries[idx];
    if (   event_match(event, &handler->match_event, &handler->match_mask)
        && handler->handler(handler, event)) {
      return True;
    }
  }
  return False;
}

void timeout_handle() {
  if (!timeout_handlers) return;
  struct timeval current_time;
  if (gettimeofday(&current_time, NULL) != 0) {
    ERROR("gettimeofday", "gettimeofday() returned error");
  }
  for (int idx = 0; idx < timeout_handlers->count; idx++) {
    TimeoutHandler *handler = (TimeoutHandler *) timeout_handlers->entries[idx];
    if (timerisset(&handler->next) && timercmp(&current_time, &handler->next, >=)) {
      handler->handler(handler, &current_time);
      if (timerisset(&handler->interval)) {
        struct timeval res;
        while (timercmp(&current_time, &handler->next, >=)) {
          timeradd(&handler->next, &handler->interval, &res);
          handler->next = res;
        }
      } else {
        timerclear(&handler->next);
      }
    }
  }
  return;
}

Bool exit_mainloop_flag = False;

void mainloop_run() {
  int display_fd = ConnectionNumber(display);
  fd_set in_fds;
  struct timeval timeout;
  XEvent e;
    
  while (!exit_mainloop_flag) {
    FD_ZERO(&in_fds);
    FD_SET(display_fd, &in_fds);
    timeout.tv_usec = 30000;
    timeout.tv_sec = 0;

    select(display_fd + 1, &in_fds, NULL, NULL, &timeout);
    timeout_handle();
    while (XPending(display)) {
      XNextEvent(display, &e);
      mainloop_event_handle(&e);
      XSync(display, False);
    }
  }
}

void mainloop_exit() {
  exit_mainloop_flag = True;
}

#include <pebble.h>
#include "main_window.h"
#include "note_window.h"
#include "load_window.h"
#include "error_window.h"

int main(void) {
  error_window_create();
  load_window_create();
  main_window_create();
  note_window_create();

  //error_window_show("App is useless...");
  Window* mainWindow = main_window_get_window();
  if(mainWindow){
    window_stack_push(main_window_get_window(), true);
  } else {
    error_window_show("Unable to show main window");
  }

  app_event_loop();

  note_window_create();
  main_window_destroy();
  load_window_destroy();
  error_window_destroy();
}

#include <pebble.h>
#include "error_window.h"

Window *errorWindow;
Layer *errorGraphicsLayer;
char currentErrorText[1][60];

void error_window_show(char *errorText){
    //Copy error text to global
    strncpy(currentErrorText[0], errorText, sizeof(currentErrorText[0]));

    if(errorGraphicsLayer){
        layer_mark_dirty(errorGraphicsLayer);
    }
    
    window_stack_push(errorWindow, true);
}

void error_graphics_proc(Layer *layer, GContext *ctx){
    GRect window_bounds = layer_get_bounds(layer);
    int midWidth = window_bounds.size.w / 2;
    int midHeight = window_bounds.size.h / 2;

    graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorWindsorTan, GColorBlack));
    graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
    //Set color of the text to draw by setting color for context.
    graphics_context_set_text_color(ctx, GColorWhite);
    //Draw text with graphics context instead of using a text layer. Final null paremter is for GTextAttributes
    graphics_draw_text(ctx, currentErrorText[0], fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), /*GRect(0,114,144,50)*/GRect(0, midHeight + 24, window_bounds.size.w, 50), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

    //Draws a face with x-ed out eyes
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_draw_circle(ctx, GPoint(/*72, 60*/midWidth, midHeight - 24), 50);
    graphics_draw_circle(ctx, GPoint(/*72, 60*/midWidth, midHeight - 24), 49);
    graphics_draw_circle(ctx, GPoint(/*72, 60*/midWidth, midHeight - 24), 48);
    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_line(ctx, GPoint(/*44, 36*/midWidth - 28, midHeight - 48), GPoint(/*58, 50*/midWidth - 14, midHeight - 34));
    graphics_draw_line(ctx, GPoint(/*58, 36*/midWidth - 14, midHeight - 48), GPoint(/*44, 50*/midWidth - 28, midHeight - 34));
    graphics_draw_line(ctx, GPoint(/*84, 36*/midWidth + 12, midHeight - 48), GPoint(/*98, 50*/midWidth + 26, midHeight - 34));
    graphics_draw_line(ctx, GPoint(/*98, 36*/midWidth + 26, midHeight - 48), GPoint(/*84, 50*/midWidth + 12, midHeight - 34));
    graphics_context_set_stroke_width(ctx, 3);
    graphics_draw_line(ctx, GPoint(/*36, 76*/midWidth - 36, midHeight - 8), GPoint(/*88, 92*/midWidth + 16, midHeight + 8));
    
}

//Called when window is placed onto window stack
void error_window_load(Window *window){
    printf("Error window loaded");
    Layer *window_layer = window_get_root_layer(window);

    errorGraphicsLayer = layer_create(layer_get_bounds(window_layer));

    //Set function to call when this layer is refreshed by the OS. Error graphics procedure
    //Called when shown, or when the layer is marked as dirty.
    layer_set_update_proc(errorGraphicsLayer, error_graphics_proc);
    
    layer_add_child(window_layer, errorGraphicsLayer);

    printf("Error window load finished");
}

//Called when removed from window stack.
void error_window_unload(Window *window){
    layer_remove_from_parent(errorGraphicsLayer);
    layer_destroy(errorGraphicsLayer);
    //Provided workaround for crash on second call of error_window_show
    errorGraphicsLayer = NULL;
}

void error_window_create(){
    printf("Error window create called");
    //Create a new window and store it in global splashWindow
    errorWindow = window_create();

    //Somehow, using a struct instead of directly passing is the cause of the crash!
    //Window handler struct
    //Contains function pointers to event handlers
    /*
    WindowHandlers handlers;
    handlers.load = splash_window_load;
    handlers.unload = splash_window_unload;
    */

    //Set up events (handlers = events for pebble)
    //Arguments: Window to set handlers for, set events we handle via function pointers
    window_set_window_handlers(errorWindow, (WindowHandlers){
        .load = error_window_load, 
        .unload = error_window_unload
    });
}

void error_window_destroy(){
    //Built in window destroy function for pebble?
    window_destroy(errorWindow);
}

Window *error_window_get_window(){
    return errorWindow;
}
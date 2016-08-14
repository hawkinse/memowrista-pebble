#include <pebble.h>
#include "main_window.h"
#include "load_window.h"

Window* loadWindow;
GBitmap* loadImage;
BitmapLayer* loadImageLayer;
TextLayer* loadTextLayer;
char* loadText = "Loading...";

void load_window_show(){
    #if !DEBUG_DUMMY_PHONE //Some dummy phone code can cause the loading screen to stick, and is so fast that it's not needed.'
    if(!window_stack_contains_window(loadWindow)){
        window_stack_push(loadWindow, true);
    }
    #else
    printf("Debug dummy phone - loading window requested to show but ignored");
    #endif
}
void load_button_back_callback(ClickRecognizerRef recognizer, void* context){
    //Simply exist so the user can't back out of the loading screen
}

void load_window_click_provider(void *context){
    window_single_click_subscribe(BUTTON_ID_BACK, load_button_back_callback);
}

//Called when window is placed onto window stack
void load_window_load(Window *window){
    printf("load window loaded");
    Layer *window_layer = window_get_root_layer(window);
    //Load load image into GBitmap. All resources start with RESOURCE_ID_ before the actual resource ID name
    //loadImage = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LOAD);

    //TODO - Make layout relative to screen center!

    //Set up bitmap splahs layer. Square pebbles are 144x168 excluding upcoming Time 2
    loadImageLayer = bitmap_layer_create(GRect(0, 0, 144, 120));
    //Set the layer to display the loaded image
    //bitmap_layer_set_bitmap(loadImageLayer, loadImage);
    //Sets how to draw the image. Our image has transparancy, so we need GCompOpSet for this. See Pebble Developer Documentation
    bitmap_layer_set_compositing_mode(loadImageLayer, GCompOpSet);
    //Attach the load image layer to the root window layer
    //Need to call bitmap_layer_get_layer because BitmapLayer is not a Layer
    layer_add_child(window_layer, bitmap_layer_get_layer(loadImageLayer));

    loadTextLayer = text_layer_create(GRect(0, 120, 144, 50));
    text_layer_set_text(loadTextLayer, loadText);
    text_layer_set_text_alignment(loadTextLayer, GTextAlignmentCenter);
    text_layer_set_font(loadTextLayer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_enable_screen_text_flow_and_paging(loadTextLayer, 2);
    layer_add_child(window_layer, text_layer_get_layer(loadTextLayer));

    window_set_click_config_provider(loadWindow, (ClickConfigProvider)load_window_click_provider);

    printf("load window load finished");
}

//Called when removed from window stack.
void load_window_unload(Window *window){
    gbitmap_destroy(loadImage);
    bitmap_layer_destroy(loadImageLayer);
    text_layer_destroy(loadTextLayer);
}

void load_window_disappear(Window *window){
    //Explicitly pull window out of the stack. 
    window_stack_remove(window, true);
}

void load_window_create(){
    printf("load window create called");
    //Create a new window and store it in global loadWindow
    loadWindow = window_create();

    //Set up events (handlers = events for pebble)
    //Arguments: Window to set handlers for, set events we handle via function pointers
    window_set_window_handlers(loadWindow, (WindowHandlers){
        .load = load_window_load, 
        .unload = load_window_unload,
        .disappear = load_window_disappear
    });

}

void load_window_destroy(){
    window_destroy(loadWindow);
}

Window *load_window_get_window(){
    return loadWindow;
}
#include <pebble.h>
#include "main_window.h"
#include "load_window.h"

Window* loadWindow;
GBitmap* loadImage;
BitmapLayer* loadImageLayer;
TextLayer* loadTextLayer;
Layer* progressBarLayer;
Layer* backgroundLayer;

char* loadText = "Loading...";
float m_progressPercentage = 0.0f;

void load_window_set_percentage(float percentage){
    m_progressPercentage = percentage;
    if(progressBarLayer != NULL){
        layer_mark_dirty(progressBarLayer);
    }
}

void load_window_show(){
    #if !DISABLE_LOAD_WINDOW
    #if !DEBUG_DUMMY_PHONE //Some dummy phone code can cause the loading screen to stick, and is so fast that it's not needed.'
    if(!window_stack_contains_window(loadWindow)){
        window_stack_push(loadWindow, true);
    }
    #else
    printf("Debug dummy phone - loading window requested to show but ignored");
    #endif
    #endif
    m_progressPercentage = 0.0f;
}

void background_layer_proc(Layer* layer, GContext* ctx){
    GRect layer_bounds = layer_get_bounds(layer);
    graphics_context_set_fill_color(ctx, COLOR_BACKGROUND);
    graphics_fill_rect(ctx, layer_bounds, 0, GCornerNone);
}

void progress_bar_proc(Layer *layer, GContext *ctx){
    GRect layer_bounds = layer_get_bounds(layer);
    int midWidth = layer_bounds.size.w / 2;
    int midHeight = layer_bounds.size.h / 2;

    graphics_context_set_fill_color(ctx, COLOR_PROGRESS_BACKGROUND);
    GRect mainBarBackgroundBounds = GRect(PROGRESS_BAR_HEIGHT / 2, 0, PROGRESS_BAR_WIDTH - (PROGRESS_BAR_HEIGHT), PROGRESS_BAR_HEIGHT);

    graphics_fill_circle(ctx, GPoint(PROGRESS_BAR_HEIGHT / 2, midHeight), PROGRESS_BAR_HEIGHT / 2);
    graphics_fill_circle(ctx, GPoint(PROGRESS_BAR_WIDTH - PROGRESS_BAR_HEIGHT, midHeight), PROGRESS_BAR_HEIGHT / 2);
    graphics_fill_rect(ctx, mainBarBackgroundBounds, 0, GCornerNone);

    graphics_context_set_fill_color(ctx, COLOR_PROGRESS_BAR);
     
    if(m_progressPercentage > 0){
        graphics_fill_circle(ctx, GPoint(PROGRESS_BAR_HEIGHT / 2, midHeight), PROGRESS_BAR_HEIGHT / 2);
        if(m_progressPercentage > 1){
            graphics_fill_circle(ctx, GPoint(PROGRESS_BAR_WIDTH - PROGRESS_BAR_HEIGHT, midHeight), PROGRESS_BAR_HEIGHT / 2);
        }
        GRect mainBarProgressBounds = mainBarBackgroundBounds;
        mainBarProgressBounds.size.w *= m_progressPercentage;
        graphics_fill_rect(ctx, mainBarProgressBounds, 0, GCornersAll);
    }
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
    #if !DISABLE_LOAD_WINDOW
    Layer *window_layer = window_get_root_layer(window);

    GRect window_bounds = layer_get_bounds(window_layer);
    int midWidth = window_bounds.size.w / 2;
    int midHeight = window_bounds.size.h / 2;

    backgroundLayer = layer_create(window_bounds);
    layer_set_update_proc(backgroundLayer, background_layer_proc);
    layer_add_child(window_layer, backgroundLayer);

    progressBarLayer = layer_create(GRect(midWidth - PROGRESS_BAR_WIDTH / 2, midHeight + 30, PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT ));
    layer_set_update_proc(progressBarLayer, progress_bar_proc);    
    layer_add_child(window_layer, progressBarLayer);

    //Load load image into GBitmap. All resources start with RESOURCE_ID_ before the actual resource ID name
    loadImage = gbitmap_create_with_resource(IMG_LOADING);

    //Set up bitmap splahs layer. Square pebbles are 144x168 excluding upcoming Time 2
    loadImageLayer = bitmap_layer_create(GRect(midWidth - 32, midHeight - 48, 64, 64));
    //Set the layer to display the loaded image
    bitmap_layer_set_bitmap(loadImageLayer, loadImage);
    //Sets how to draw the image. Our image has transparancy, so we need GCompOpSet for this. See Pebble Developer Documentation
    bitmap_layer_set_compositing_mode(loadImageLayer, GCompOpSet);
    //Attach the load image layer to the root window layer
    //Need to call bitmap_layer_get_layer because BitmapLayer is not a Layer
    layer_add_child(window_layer, bitmap_layer_get_layer(loadImageLayer));

    loadTextLayer = text_layer_create(GRect(0, 120 + PROGRESS_BAR_HEIGHT, 144, 50 - PROGRESS_BAR_HEIGHT));
    text_layer_set_text(loadTextLayer, loadText);
    text_layer_set_text_alignment(loadTextLayer, GTextAlignmentCenter);
    text_layer_set_font(loadTextLayer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_background_color(loadTextLayer, COLOR_BACKGROUND);
    text_layer_set_text_color(loadTextLayer, COLOR_LOADING_TEXT);
    layer_add_child(window_layer, text_layer_get_layer(loadTextLayer));
    text_layer_enable_screen_text_flow_and_paging(loadTextLayer, 2);

    window_set_click_config_provider(loadWindow, (ClickConfigProvider)load_window_click_provider);
    #endif
}

//Called when removed from window stack.
void load_window_unload(Window *window){
    #if !DISABLE_LOAD_WINDOW
    printf("Loading window is unloading");
    layer_remove_from_parent(text_layer_get_layer(loadTextLayer));
    layer_remove_from_parent(progressBarLayer);
    layer_remove_from_parent(bitmap_layer_get_layer(loadImageLayer));
    layer_remove_from_parent(backgroundLayer);

    layer_destroy(progressBarLayer);
    gbitmap_destroy(loadImage);
    bitmap_layer_destroy(loadImageLayer);
    text_layer_destroy(loadTextLayer);
    layer_destroy(backgroundLayer);

    progressBarLayer = NULL;
    loadImage = NULL;
    loadImageLayer = NULL;
    loadTextLayer = NULL;
    backgroundLayer = NULL;
    #endif
}

void load_window_disappear(Window *window){
    printf("load_window_disappear");
    //Explicitly pull window out of the stack. 
    window_stack_remove(window, true);
}

void load_window_create(){
    printf("load window create called");
    //Create a new window and store it in global loadWindow
    loadWindow = window_create();
    #if !DISABLE_LOAD_WINDOW
    //Set up events (handlers = events for pebble)
    //Arguments: Window to set handlers for, set events we handle via function pointers
    window_set_window_handlers(loadWindow, (WindowHandlers){
        .load = load_window_load, 
        .unload = load_window_unload,
        .disappear = load_window_disappear
    });
    #endif

}

void load_window_destroy(){
    window_destroy(loadWindow);
}

Window *load_window_get_window(){
    return loadWindow;
}
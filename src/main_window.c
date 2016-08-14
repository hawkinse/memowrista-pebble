#include <pebble.h>
#include "error_window.h"
#include "load_window.h"
#include "note_window.h"
#include "main_window.h"

Window* mainWindow;
StatusBarLayer* mainTimeStatusBar;
MenuLayer *mainMenuLayer;
GBitmap* loadingBitmap;
Layer* loadingLayer;

NoteHeader* noteHeaders;
uint32_t noteCount = 0;
uint32_t recievedNoteCount = 0;
bool bAutoEnterFirst = false;

#if DEBUG_DUMMY_PHONE
char dummyNoteBody[NOTE_BODY_SIZE] = "Dummy phone default body!";
#endif

//NOTE FUNCS
void delete_note_headers(){
    if(noteHeaders){
        /*
        for(uint32_t i = noteCount; i > 0; i--){
            free(noteHeaders[i - 1]);
        }
        free(noteHeaders);
        */

        //Think this is sufficient?
        free(noteHeaders);
    }
    noteCount = 0;
}

#if DEBUG_MODE
void generate_dummy_note_headers(uint32_t count){
    delete_note_headers();
    //Allocate memory for headers
    noteCount = count;
    noteHeaders = malloc(sizeof(NoteHeader) * noteCount);

    //Fill with dummy data
    for(uint32_t i = 0; i < noteCount; i++){
        noteHeaders[i].id = i;
        strncpy(noteHeaders[i].title, "Dummy note", sizeof(noteHeaders[i]));
    }

    if(mainMenuLayer){
        menu_layer_reload_data(mainMenuLayer);
    }
}
#endif

void send_note_request(NoteAppMessageKey msg, int32_t value){
    #if !DEBUG_DUMMY_PHONE
    DictionaryIterator *outDict;
    AppMessageResult outResult = app_message_outbox_begin(&outDict);
    switch(outResult){
        case APP_MSG_OK:
            dict_write_int(outDict, msg, &value, sizeof(int32_t), true);
            outResult = app_message_outbox_send();
            if(outResult == APP_MSG_OK){
                APP_LOG(APP_LOG_LEVEL_INFO, "Sent pebble request %d", msg);
            } else {
                APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending request %d, result %d", (int)msg, (int)outResult);
                //error_window_show("Unable to send data to phone!");
            }
            break;
        case APP_MSG_SEND_TIMEOUT:
        case APP_MSG_BUSY:
            APP_LOG(APP_LOG_LEVEL_ERROR, "Timeout or busy sending msg %d, result %d.", (int)msg, (int)outResult);
            break;
    	default:
            APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox before pebble request %d, result %d",(int)msg, (int)outResult);
            error_window_show("Unable to send data to phone!");
    }
    #else
    switch(msg){
        case MSG_PEBBLE_REQUEST_NOTE_COUNT:
            printf("Dummy phone mode - setting note count to 1");
            response_set_note_count(1);
            break;
        case MSG_PEBBLE_REQUEST_NOTE_ID:
            printf("Dummy phone mode - setting note id to 1");
            response_set_current_note_id(1);
            break;
        case MSG_PEBBLE_REQUEST_NOTE_TITLE:
            printf("Dummy phone mode - setting note default title");
            response_set_current_note_title("Dummy phone default title!");
            break;
        case MSG_PEBBLE_REQUEST_NOTE_BODY:
            printf("Dummy phone mode - setting note default body");
            response_set_note_body(dummyNoteBody);
            break;
        case MSG_PEBBLE_REQUEST_NOTE_TIME:
            printf("Dummy phone mode - setting note timestamp to 0");
            response_set_note_timestamp(0);
            break;
        case MSG_PEBBLE_DELETE_NOTE:
            printf("Dummy phone mode - delete note. Ignoring request!");
            error_window_show("Dummy phone note delete");
            break;
        default:
            printf("Dummy phone mode - note request %d unimplemented!", msg);
    }
    #endif
}

void send_note_edit(NoteAppMessageKey msg, int32_t id, char* edit){
    #if !DEBUG_DUMMY_PHONE
    DictionaryIterator *outDict;
    AppMessageResult outResult = app_message_outbox_begin(&outDict);
    switch(outResult){
        case APP_MSG_OK:
            dict_write_int(outDict, MSG_PEBBLE_SET_EDIT_ID, &id, sizeof(int32_t), true);
            dict_write_cstring(outDict, msg, edit);
            outResult = app_message_outbox_send();
            if(outResult == APP_MSG_OK){
                APP_LOG(APP_LOG_LEVEL_INFO, "Sent pebble request %d", msg);
                bAutoEnterFirst = true;
            } else {
                APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending request %d, result %d", (int)msg, (int)outResult);
                error_window_show("Unable to send data to phone!");
            }
            break;
        case APP_MSG_SEND_TIMEOUT:
        case APP_MSG_BUSY:
            APP_LOG(APP_LOG_LEVEL_ERROR, "Timeout or busy sending msg %d, result %d.", (int)msg, (int)outResult);
            break;
    	default:
            APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox before pebble request %d, result %d",(int)msg, (int)outResult);
            error_window_show("Unable to send data to phone!");
    }
    #else
    switch(msg){
        case MSG_PEBBLE_REPLACE_TITLE:
            printf("Dummy phone mode - replacing title of dummy note");
            strncpy(noteHeaders[0].title, edit, sizeof(noteHeaders[0].title));
            break;
        case MSG_PEBBLE_REPLACE_BODY:
            printf("Dummy phone mode - replacing body of dummy note");
            strncpy(dummyNoteBody, edit, sizeof(dummyNoteBody));
            break;
        case MSG_PEBBLE_APPEND_BODY:
            printf("Dummy phone mode - append body of dummy note");
            strcat(dummyNoteBody, edit);
            //strncpy(dummyNoteBody + strlen(edit), edit, sizeof(dummyNoteBody - strlen(edit)));
            break;
        default:
            printf("Dummy phone mode - note edit %d unimplemented!", msg);
    }

    //Instead of re-requesting notes, just enter first note as otherwise we'll overwrite changes!
    note_set_body(&noteHeaders[0], dummyNoteBody);
    window_stack_push(note_window_get_window(), true);
    #endif
}

void request_new_note(){
    send_note_request(MSG_PEBBLE_NEW_NOTE, 0);
    bAutoEnterFirst = true;
}

void request_notes(){
    recievedNoteCount = 0;
    send_note_request(MSG_PEBBLE_REQUEST_NOTE_COUNT, 0);
}

void response_set_note_count(int32_t count){
    //Push loading screen. Do this here instead of request_notes since request_notes is called from load callback. Pushing a window during load will crash.
    load_window_show();

    delete_note_headers();
    noteCount = count;
    APP_LOG(APP_LOG_LEVEL_INFO, "About to allocate %d bytes for note headers", (int)(sizeof(NoteHeader) * /*noteCount*/count));
    noteHeaders = malloc(sizeof(NoteHeader) * noteCount);
    APP_LOG(APP_LOG_LEVEL_INFO, "Note count set to %d from %d", (int)noteCount, (int)count);

    if(/*noteCount*/count > 0){
        //Send a reqest for the next ID, using the recieved note count as an index on the Android side
        send_note_request(MSG_PEBBLE_REQUEST_NOTE_ID, recievedNoteCount);
    }
}

void response_set_current_note_id(int32_t id){
    APP_LOG(APP_LOG_LEVEL_INFO, "About to attempt setting ID of index %d to %d", (int)recievedNoteCount, (int)id);
    noteHeaders[recievedNoteCount].id = id;
    APP_LOG(APP_LOG_LEVEL_INFO, "ID of index %d set to %d", (int)recievedNoteCount, (int)id);
    //Send a request for the title corresponding to this id
    send_note_request(MSG_PEBBLE_REQUEST_NOTE_TITLE, id);
}


void response_set_current_note_title(char* title){
    strncpy(noteHeaders[recievedNoteCount].title, title, sizeof(noteHeaders[recievedNoteCount].title));
    APP_LOG(APP_LOG_LEVEL_INFO, "Set title of index %d", (int)recievedNoteCount);
    
    printf("Response set current note title");

    recievedNoteCount++;
    if(recievedNoteCount < noteCount){
        send_note_request(MSG_PEBBLE_REQUEST_NOTE_ID, recievedNoteCount);
    } else {
        APP_LOG(APP_LOG_LEVEL_INFO, "Final message recieved!");
        if(mainMenuLayer){
            menu_layer_reload_data(mainMenuLayer);
            menu_layer_set_selected_index(mainMenuLayer, (MenuIndex){0, 0}, MenuRowAlignNone, true);

            //Actually scroll so the selected index is in view
            ScrollLayer* menuScrollLayer = menu_layer_get_scroll_layer(mainMenuLayer);
            scroll_layer_set_content_offset(menuScrollLayer, (GPoint){0,0}, false);

            //Automatically enter the first note if true. Used when a new note is made from the watch.
            if(bAutoEnterFirst){
                send_note_request(MSG_PEBBLE_REQUEST_NOTE_BODY, noteHeaders[0].id);
                bAutoEnterFirst = false;
            } else {
                printf("Removing window from stack");
                window_stack_remove(load_window_get_window(), true);
            }
        } else {
            APP_LOG(APP_LOG_LEVEL_ERROR, "No main menu layer!");
        }
    }
}

void response_set_note_body(char* body){
    MenuIndex selectedMenuItem = menu_layer_get_selected_index(mainMenuLayer);
    note_set_body(&noteHeaders[selectedMenuItem.row], body);
    send_note_request(MSG_PEBBLE_REQUEST_NOTE_TIME, noteHeaders[selectedMenuItem.row].id);
}

void response_set_note_timestamp(int32_t timestamp){
    note_set_modified_timestamp(timestamp);
    window_stack_remove(load_window_get_window(), true);
    window_stack_push(note_window_get_window(), true);
}

//APP MESSAGE FUNCS

void process_tuple(Tuple *t){
    int key = t->key;
    APP_LOG(APP_LOG_LEVEL_INFO, "Got key %d", key);

    switch(key){
        case MSG_PHONE_SEND_NOTE_COUNT:
            response_set_note_count(t->value->int32);
            break;
        case MSG_PHONE_SEND_NOTE_ID:
            response_set_current_note_id(t->value->int32);
            break;
        case MSG_PHONE_SEND_NOTE_TITLE:
            response_set_current_note_title(t->value->cstring);
            break;
        case MSG_PHONE_SEND_NOTE_BODY:
            response_set_note_body(t->value->cstring);
            break;
        case MSG_PHONE_SEND_NOTE_TIME:
            response_set_note_timestamp(t->value->int32);
            break;
        case MSG_PHONE_UPDATED:
            request_notes();
            break;
        case MSG_PHONE_GENERIC_ERROR:
            error_window_show("Phone side generic error");
        default:
            APP_LOG(APP_LOG_LEVEL_ERROR, "Recieved unknown key %d with (presumed int) value %d", key, (int)t->value->int32);
    }
}

void message_inbox(DictionaryIterator *iter, void *context){
    Tuple *t = dict_read_first(iter);
    if(t){
        process_tuple(t);
    }
    while(t != NULL){
        t = dict_read_next(iter);
        if(t){
            process_tuple(t);
        }
    }
}

void message_inbox_dropped(AppMessageResult reason, void *context){
    APP_LOG(APP_LOG_LEVEL_INFO, "Message dropped, reason %d.", reason);
}

//MAIN MENU FUNCS
uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
    //First section is the notes themselves
    //Second section is actions.
    return 2;
}

uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
    switch(section_index){
        case 0:
            return noteCount; //Test value
        case 1:
            return 2;
        default:
            return 0;
    }
    return 0;
}

int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
    return MENU_CELL_BASIC_HEADER_HEIGHT;
}

void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
    graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorWindsorTan, GColorBlack));
    graphics_context_set_text_color(ctx, GColorWhite);

    graphics_fill_rect(ctx, layer_get_bounds(cell_layer), 0, GCornerNone);

    GRect layerBounds = layer_get_bounds(cell_layer);
    layerBounds.origin.y -= 5;
    switch(section_index){
        case 0:            
            //Draw text with graphics context instead of using a text layer. Final null paremter is for GTextAttributes
            graphics_draw_text(ctx, "Notes", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), layerBounds, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
            //Basic function built in for drawing text on header?
            //menu_cell_basic_header_draw(ctx, cell_layer, "Notes");
            break;
        case 1:
            graphics_draw_text(ctx, "Actions", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), layerBounds, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
            //menu_cell_basic_header_draw(ctx, cell_layer, "Actions");
            break;
    }
}

void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
    GRect layerBounds = layer_get_bounds(cell_layer);
    layerBounds.origin.y += 5;
    layerBounds.origin.x += 5;
    #if PBL_RECT
    layerBounds.size.w -= 5;
    #endif
    switch(cell_index->section){
        case 0:
            //menu_cell_basic_draw(ctx, cell_layer, "Menu Test", NULL, NULL); //Last argument is an icon bitmap
            graphics_draw_text(ctx, noteHeaders[cell_index->row].title, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), layerBounds, GTextOverflowModeTrailingEllipsis, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft), NULL);
            /*
            switch(cell_index->row){
                case 0:
                    //Basic function built in for drawing text on row?
                    menu_cell_basic_draw(ctx, cell_layer, "Example with a really long title name", "40 degrees", NULL); //Last argument is an icon bitmap
                    break;
            }
            */
            break;
        case 1:
            switch(cell_index->row){
                case 0:
                    graphics_draw_text(ctx, "New Note", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), layerBounds, GTextOverflowModeTrailingEllipsis, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft), NULL);
                    //menu_cell_basic_draw(ctx, cell_layer, "+", NULL, NULL); //Last argument is an icon bitmap
                    break;
                case 1:
                    graphics_draw_text(ctx, "Refresh Note List", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), layerBounds, GTextOverflowModeTrailingEllipsis, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft), NULL);
                    //menu_cell_basic_draw(ctx, cell_layer, "+", NULL, NULL); //Last argument is an icon bitmap
                    break;

            }
            break;
    }
}

void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
	//Called when a menu item is clicked with select button. 
    char* selectedString = malloc(sizeof(char) * 60);
    switch(cell_index->section){
        case 0:            
            snprintf(selectedString, sizeof(selectedString), "Existing note %d selected!", cell_index->row);
            printf("Cell index: %d", cell_index->row);
            /*
            note_set_contents(&noteHeaders[cell_index->row], "This is long body text that I'm hoping is long enough to require wrapping so I can test proper scrolling with dynamic text. Apparently I need to make it just a little bit longer before it can scroll, hopefully adding this next sentence does the trick. I cant wait to remove this string, it makes for poor code readability but it's too temporary to justify making a static var for it.", "1970/01/01", "12:00:00 AM");
            window_stack_push(note_window_get_window(), true);
            */
            send_note_request(MSG_PEBBLE_REQUEST_NOTE_BODY, noteHeaders[cell_index->row].id);
            break;
        case 1:
            switch(cell_index->row){
                case 0:
                    request_new_note();
                    break;
                case 1:
                    request_notes();
            }
            break;
    }
    free(selectedString);
    //Temporary... clicking a row generates an error window
    //error_window_show("Error message");    
}


// WINDOW FUNCS 

void main_window_round_background_update_proc(Layer* layer, GContext *ctx){
    graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorWindsorTan, GColorBlack));
    graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

//Called when window is placed onto window stack
void main_window_load(Window *window){
    Layer* window_layer = window_get_root_layer(window);

    //Start app message. Maybe move this to its own window or module?
    //Important to have this run 
    app_message_register_inbox_received(message_inbox);
    app_message_register_inbox_dropped(message_inbox_dropped);
    //256 is size of inbox and outbox.
    app_message_open(2048, 2048);
   
    //Old position... race condition! Will fail if the phone responds before the main menu layer is created!
    /*
    //Request notes from phone
    if(recievedNoteCount != noteCount || noteCount == 0){
        request_notes();
    }
    */

    if(!mainTimeStatusBar){
        mainTimeStatusBar = status_bar_layer_create();
    }

    //status_bar_layer_set_colors(timeStatusBar, PBL_IF_COLOR_ELSE(GColorRajah, GColorDarkGray), GColorWhite);
    #if PBL_COLOR
    status_bar_layer_set_colors(mainTimeStatusBar, GColorWindsorTan, GColorWhite);
    #endif   

    //errorGraphicsLayer = layer_create(layer_get_bounds(window_layer));
    layer_add_child(window_layer, status_bar_layer_get_layer(mainTimeStatusBar));

    //generate_dummy_note_headers(5);

    if(!mainMenuLayer){
    GRect window_bounds = layer_get_bounds(window_layer);
    mainMenuLayer = menu_layer_create(GRect(0, STATUS_BAR_LAYER_HEIGHT, window_bounds.size.w, window_bounds.size.h - STATUS_BAR_LAYER_HEIGHT));
    menu_layer_set_highlight_colors(mainMenuLayer, PBL_IF_COLOR_ELSE(GColorRajah, GColorDarkGray), GColorWhite);
    menu_layer_set_callbacks(mainMenuLayer, NULL, (MenuLayerCallbacks){
        .get_num_sections = menu_get_num_sections_callback,
        .get_num_rows = menu_get_num_rows_callback,
        .get_header_height = menu_get_header_height_callback,
        .draw_header = menu_draw_header_callback,
        .draw_row = menu_draw_row_callback,
        .select_click = menu_select_callback
    });
    }

    printf("Main menu layer: %p", mainMenuLayer);

    menu_layer_set_click_config_onto_window(mainMenuLayer, window);
    layer_add_child(window_layer, menu_layer_get_layer(mainMenuLayer));

    if(recievedNoteCount != noteCount || noteCount == 0){
        request_notes();
    }
}

//Called when removed from window stack.
void main_window_unload(Window *window){
    //layer_destroy(errorGraphicsLayer);
    //Provided workaround for crash on second call of error_window_show
    //errorGraphicsLayer = NULL;

    delete_note_headers();
    menu_layer_destroy(mainMenuLayer);
    status_bar_layer_destroy(mainTimeStatusBar);
}

void main_window_create(){
    //Create a new window and store it in global splashWindow
    mainWindow = window_create();

    if(!mainWindow){
        error_window_show("Unable to create main window");
        return;
    }

    //Set up events (handlers = events for pebble)
    //Arguments: Window to set handlers for, set events we handle via function pointers
    window_set_window_handlers(mainWindow, (WindowHandlers){
        .load = main_window_load, 
        .unload = main_window_unload
    });
}

void main_window_destroy(){
    //Built in window destroy function for pebble?
    window_destroy(mainWindow);
}

Window* main_window_get_window(){
    return mainWindow;
}
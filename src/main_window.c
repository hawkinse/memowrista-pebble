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
char dummyNoteBody[NOTE_BODY_SIZE] = TEXT_DUMMY_PHONE_DEFAULT_BODY;
#endif

//PLACEHOLDER LOCATION - MOVE TO OWN FILE IN FUTURE!
void log_message(AppLogLevel level, char* msg){
    #if DEBUG_LOGGING
    APP_LOG(level, msg);
    #endif
}

//PLACEHOLDER LOCATION - MOVE TO OWN FILE IN FUTURE!
void log_message_int(AppLogLevel level, char* msg, int val){
    #if DEBUG_LOGGING
    APP_LOG(level, msg, val);
    #endif
}

//PLACEHOLDER LOCATION - MOVE TO OWN FILE IN FUTURE!
void log_message_string(AppLogLevel level, char* msg, char* val){
    #if DEBUG_LOGGING
    APP_LOG(level, msg, val);
    #endif
}

//NOTE FUNCS
void delete_note_headers(){
    if(noteHeaders){
        /*
        for(uint32_t i = noteCount; i > 0; i--){
            free(&noteHeaders[i - 1]);
        }
        //free(noteHeaders);
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
        strncpy(noteHeaders[i].title, TEXT_DUMMY_DEFAULT_TITLE, sizeof(noteHeaders[i]));
    }

    if(mainMenuLayer){
        menu_layer_reload_data(mainMenuLayer);
    }
}
#endif


bool com_version_check(int version){
    if(version != VERSION_COM){
        if(version > VERSION_COM){
            error_window_show(TEXT_ERROR_UPDATE_PEBBLE);
        } else {
            error_window_show(TEXT_ERROR_UPDATE_PHONE);
        }

        window_stack_remove(mainWindow, true);
    }

    return (version == VERSION_COM);
}

void send_note_request(NoteAppMessageKey msg, int32_t value){
    #if !DEBUG_DUMMY_PHONE
    DictionaryIterator *outDict;
    AppMessageResult outResult = app_message_outbox_begin(&outDict);
    switch(outResult){
        case APP_MSG_OK:
            dict_write_int(outDict, msg, &value, sizeof(int32_t), true);
            outResult = app_message_outbox_send();
            if(outResult == APP_MSG_OK){
                log_message_int(APP_LOG_LEVEL_INFO, "Sent pebble request %d", msg);
            } else {
                log_message_int(APP_LOG_LEVEL_ERROR, "Error sending request %d,", (int)msg);
                log_message_int(APP_LOG_LEVEL_ERROR, "result %d", (int)outResult);
                //error_window_show(TEXT_ERROR_SEND_FAILED);
            }
            break;
        case APP_MSG_SEND_TIMEOUT:
        case APP_MSG_BUSY:
            log_message_int(APP_LOG_LEVEL_ERROR, "Timeout or busy sending msg %d,", (int)msg);
            log_message_int(APP_LOG_LEVEL_ERROR, "result %d.", (int)outResult);
            break;
    	default:
            log_message_int(APP_LOG_LEVEL_ERROR, "Error preparing the outbox before pebble request %d,", (int)msg);
            log_message_int(APP_LOG_LEVEL_ERROR, " result %d", (int)outResult);
            error_window_show(TEXT_ERROR_SEND_FAILED);
    }
    #elif DEBUG_DUMMY_PHONE
    switch(msg){
        case MSG_PEBBLE_REQUEST_NOTE_COUNT:
            log_message(APP_LOG_LEVEL_INFO, "Dummy phone mode - setting note count to 1");
            response_set_note_count(1);
            break;
        case MSG_PEBBLE_REQUEST_NOTE_ID:
            log_message(APP_LOG_LEVEL_INFO, "Dummy phone mode - setting note id to 1");
            response_set_current_note_id(1);
            break;
        case MSG_PEBBLE_REQUEST_NOTE_TITLE:
            log_message(APP_LOG_LEVEL_INFO, "Dummy phone mode - setting note default title");
            response_set_current_note_title(TEXT_DUMMY_DEFAULT_TITLE);
            break;
        case MSG_PEBBLE_REQUEST_NOTE_BODY:
            log_message(APP_LOG_LEVEL_INFO, "Dummy phone mode - setting note body");
            response_set_note_body(dummyNoteBody);
            break;
        case MSG_PEBBLE_REQUEST_NOTE_TIME:
            log_message(APP_LOG_LEVEL_INFO, "Dummy phone mode - setting note timestamp to 0");
            response_set_note_timestamp(DUMMY_PHONE_DEFAULT_TIMESTAMP);
            break;
        case MSG_PEBBLE_DELETE_NOTE:
            log_message(APP_LOG_LEVEL_INFO, "Dummy phone mode - delete note. Ignoring request!");
            error_window_show(TEXT_DUMMY_PHONE_DELETE);
            break;
        case MSG_PEBBLE_REQUEST_COM_VERSION:
            log_message_int(APP_LOG_LEVEL_INFO, "Dummy phone mode - setting version to DUMMY_PHONE_COM_VERSION %d", DUMMY_PHONE_COM_VERSION);
            request_notes_if_compatible(DUMMY_PHONE_COM_VERSION);
        default:
            log_message_int(APP_LOG_LEVEL_INFO, "Dummy phone mode - note request %d unimplemented!", msg);
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
                log_message_int(APP_LOG_LEVEL_INFO, "Sent pebble request %d", msg);
                bAutoEnterFirst = true;
            } else {
                log_message_int(APP_LOG_LEVEL_ERROR, "Error sending request %d,", (int)msg);
                log_message_int(APP_LOG_LEVEL_ERROR, " result %d", (int)outResult);
                error_window_show(TEXT_ERROR_SEND_FAILED);
            }
            break;
        case APP_MSG_SEND_TIMEOUT:
        case APP_MSG_BUSY:
            log_message_int(APP_LOG_LEVEL_ERROR, "Timeout or busy sending msg %d,", (int)msg);
            log_message_int(APP_LOG_LEVEL_ERROR, "result %d", (int)outResult);
            break;
    	default:
            log_message_int(APP_LOG_LEVEL_ERROR, "Error preparing the outbox before pebble request %d,",(int)msg);
            log_message_int(APP_LOG_LEVEL_ERROR, " result %d", (int)outResult);
            error_window_show(TEXT_ERROR_SEND_FAILED);
    }
    #elif DEBUG_DUMMY_PHONE
    switch(msg){
        case MSG_PEBBLE_REPLACE_TITLE:
            log_message(APP_LOG_LEVEL_INFO, "Dummy phone mode - replacing title of dummy note");
            strncpy(noteHeaders[0].title, edit, sizeof(noteHeaders[0].title));
            break;
        case MSG_PEBBLE_REPLACE_BODY:
            log_message(APP_LOG_LEVEL_INFO, "Dummy phone mode - replacing body of dummy note");
            strncpy(dummyNoteBody, edit, sizeof(dummyNoteBody));
            break;
        case MSG_PEBBLE_APPEND_BODY:
            log_message(APP_LOG_LEVEL_INFO, "Dummy phone mode - append body of dummy note");
            strcat(dummyNoteBody, "\n");
            strcat(dummyNoteBody, edit);
            break;
        default:
            log_message_int(APP_LOG_LEVEL_ERROR, "Dummy phone mode - note edit %d unimplemented!", msg);
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

void request_notes_if_compatible(int version){
    if(com_version_check(version)){
        request_notes();
    }
}

void response_set_note_count(int32_t count){
    //Push loading screen. Do this here instead of request_notes since request_notes is called from load callback. Pushing a window during load will crash.
    load_window_show();
    
    log_message_int(APP_LOG_LEVEL_INFO, "Bytes free before deleteing and reallocating headers: %d", (int)heap_bytes_free());
    
    delete_note_headers();
    noteCount = count;
    log_message_int(APP_LOG_LEVEL_INFO, "About to allocate %d bytes for note headers", (int)(sizeof(NoteHeader) * /*noteCount*/count));
    noteHeaders = malloc(sizeof(NoteHeader) * noteCount);
    log_message_int(APP_LOG_LEVEL_INFO, "Note count set to %d", (int)noteCount);
    log_message_int(APP_LOG_LEVEL_INFO, "from %d", (int)count);

    log_message_int(APP_LOG_LEVEL_INFO, "Bytes free after deleteing and reallocating headers: %d", (int)heap_bytes_free());

    if(/*noteCount*/count > 0){
        //Send a reqest for the next ID, using the recieved note count as an index on the Android side
        send_note_request(MSG_PEBBLE_REQUEST_NOTE_ID, recievedNoteCount);
    }
}

void response_set_current_note_id(int32_t id){
    log_message_int(APP_LOG_LEVEL_INFO, "About to attempt setting ID of index %d to %d", (int)recievedNoteCount);
    log_message_int(APP_LOG_LEVEL_INFO, "to %d", (int)id);
    noteHeaders[recievedNoteCount].id = id;
    log_message_int(APP_LOG_LEVEL_INFO, "ID of index %d", (int)recievedNoteCount);
    log_message_int(APP_LOG_LEVEL_INFO, "set to %d", (int)id);
    //Send a request for the title corresponding to this id
    send_note_request(MSG_PEBBLE_REQUEST_NOTE_TITLE, id);
}


void response_set_current_note_title(char* title){
    strncpy(noteHeaders[recievedNoteCount].title, title, sizeof(noteHeaders[recievedNoteCount].title));
    log_message_int(APP_LOG_LEVEL_INFO, "Set title of index %d", (int)recievedNoteCount);

    recievedNoteCount++;
    load_window_set_percentage((float)recievedNoteCount / (float)noteCount);

    if(recievedNoteCount < noteCount){
        send_note_request(MSG_PEBBLE_REQUEST_NOTE_ID, recievedNoteCount);
    } else {
        log_message(APP_LOG_LEVEL_INFO, "Final message recieved!");
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
                log_message(APP_LOG_LEVEL_INFO, "Removing load window from stack");
                window_stack_remove(load_window_get_window(), true);
            }
        } else {
            log_message(APP_LOG_LEVEL_ERROR, "No main menu layer!");
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
    log_message_int(APP_LOG_LEVEL_INFO, "Got key %d", key);

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
        case MSG_PHONE_SEND_COM_VERSION:
            request_notes_if_compatible(t->value->int32);
            break;
        case MSG_PHONE_GENERIC_ERROR:
            error_window_show(TEXT_ERROR_PHONE_GENERIC);
        default:
            log_message_int(APP_LOG_LEVEL_ERROR, "Recieved unknown key %d", key);
            log_message_int(APP_LOG_LEVEL_ERROR, " with (presumed int) value %d", (int)t->value->int32);
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
    log_message_int(APP_LOG_LEVEL_INFO, "Message dropped, reason %d.", reason);
}

//MAIN MENU FUNCS
uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
    //First section is the notes themselves
    //Second section is actions.
    //Third is about and enabled debug flags
    return 3;
}

uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
    switch(section_index){
        case MENU_SECTION_NOTES:
            return noteCount;
        case MENU_SECTION_ACTIONS:
            //Return 2 for refresh and new note if in debug mode or has a mic. Return 1 for refresh otherwise.
            return PBL_IF_MICROPHONE_ELSE(2, DEBUG_MODE ? 2 : 1);
        case MENU_SECTION_ABOUT:
            //Return 1 row if not in debug mode for about. Return 4 if in debug mode so we can show debug mode options
            return (DEBUG_MODE ? 4 : 1);
        default:
            return 0;
    }
    return 0;
}

int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
    return MENU_CELL_BASIC_HEADER_HEIGHT;
}

void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
    GRect layerBounds = layer_get_bounds(cell_layer);
    layerBounds.origin.y -= 5;

    graphics_context_set_fill_color(ctx, COLOR_PRIMARY);
    graphics_context_set_text_color(ctx, COLOR_TEXT_LIGHT);

    graphics_fill_rect(ctx, layer_get_bounds(cell_layer), 0, GCornerNone);

    switch(section_index){
        case MENU_SECTION_NOTES:            
            //Draw text with graphics context instead of using a text layer. Final null paremter is for GTextAttributes
            graphics_draw_text(ctx, (noteCount > 0 ? TEXT_MENU_HEADER_NOTES : TEXT_MENU_HEADER_NOTES_NONE), fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), layerBounds, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
            //Basic function built in for drawing text on header?
            //menu_cell_basic_header_draw(ctx, cell_layer, "Notes");
            break;
        case MENU_SECTION_ACTIONS:
            graphics_draw_text(ctx, TEXT_MENU_HEADER_ACTIONS, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), layerBounds, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
            //menu_cell_basic_header_draw(ctx, cell_layer, "Actions");
            break;
        case MENU_SECTION_ABOUT:
            graphics_draw_text(ctx, TEXT_MENU_HEADER_ABOUT, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), layerBounds, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
            break;
    }

    //Add dots onto headers to distinguish headers from time and selected menu item
    graphics_context_set_stroke_color(ctx, COLOR_TEXT_LIGHT);
    for(int i = 0; i < layerBounds.size.w; i += 2){
        GPoint top;
        GPoint bottom;
        top.x = i;
        top.y = 0;
        //Shift bottom 1px to the right so it looks better if there are no notes and two headers are aligned
        bottom.x = i + 1;
        bottom.y = layerBounds.size.h - 1;
        graphics_draw_pixel(ctx, top);
        graphics_draw_pixel(ctx, bottom);
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
        case MENU_SECTION_NOTES:
            //menu_cell_basic_draw(ctx, cell_layer, "Menu Test", NULL, NULL); //Last argument is an icon bitmap
            graphics_draw_text(ctx, noteHeaders[cell_index->row].title, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), layerBounds, GTextOverflowModeTrailingEllipsis, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft), NULL);
            break;
        case MENU_SECTION_ACTIONS:
            switch(cell_index->row){
                case MENU_INDEX_ACTION_NEW_NOTE:
                    graphics_draw_text(ctx, TEXT_MENU_CONTENT_NEW_NOTE, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), layerBounds, GTextOverflowModeTrailingEllipsis, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft), NULL);
                    //menu_cell_basic_draw(ctx, cell_layer, "+", NULL, NULL); //Last argument is an icon bitmap
                    break;
                case MENU_INDEX_ACTION_REFRESH:
                    graphics_draw_text(ctx, TEXT_MENU_CONTENT_REFRESH, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), layerBounds, GTextOverflowModeTrailingEllipsis, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft), NULL);
                    //menu_cell_basic_draw(ctx, cell_layer, "+", NULL, NULL); //Last argument is an icon bitmap
                    break;

            }
            break;
        case MENU_SECTION_ABOUT:
            switch(cell_index->row){
                case MENU_INDEX_ABOUT_ABOUT:
                    menu_cell_basic_draw(ctx, cell_layer, TEXT_MENU_CONTENT_VERSION, versionString, NULL);
                    break;
                case MENU_INDEX_ABOUT_DEBUG:
                    menu_cell_basic_draw(ctx, cell_layer, TEXT_MENU_CONTENT_DEBUG, (DEBUG_MODE ? TEXT_ENABLED : TEXT_DISABLED), NULL);
                    break;
                case MENU_INDEX_ABOUT_DUMMY_MIC:
                    menu_cell_basic_draw(ctx, cell_layer, TEXT_MENU_CONTENT_DEBUG_MIC, ((PBL_IF_MICROPHONE_ELSE(false, true) && DEBUG_DUMMY_MIC) ? TEXT_ENABLED : TEXT_DISABLED), NULL);
                    break;
                case MENU_INDEX_ABOUT_DUMMY_PHONE:
                    menu_cell_basic_draw(ctx, cell_layer, TEXT_MENU_CONTENT_DEBUG_PHONE, (DEBUG_DUMMY_PHONE ? TEXT_ENABLED : TEXT_DISABLED), NULL);
                    break;
            }
    }
}

void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
	//Called when a menu item is clicked with select button. 
    log_message(APP_LOG_LEVEL_INFO, "main menu select callback");
    //log_message_int(APP_LOG_LEVEL_INFO, "Free bytes: %d", heap_bytes_free());
    switch(cell_index->section){
        case 0:            
            log_message_int(APP_LOG_LEVEL_INFO, "Cell index: %d", cell_index->row);
            /*
            note_set_contents(&noteHeaders[cell_index->row], "This is long body text that I'm hoping is long enough to require wrapping so I can test proper scrolling with dynamic text. Apparently I need to make it just a little bit longer before it can scroll, hopefully adding this next sentence does the trick. I cant wait to remove this string, it makes for poor code readability but it's too temporary to justify making a static var for it.", "1970/01/01", "12:00:00 AM");
            window_stack_push(note_window_get_window(), true);
            */
            send_note_request(MSG_PEBBLE_REQUEST_NOTE_BODY, noteHeaders[cell_index->row].id);
            break;
        case 1:
            switch(cell_index->row){
                case MENU_INDEX_ACTION_NEW_NOTE:
                    request_new_note();
                    break;
                case MENU_INDEX_ACTION_REFRESH:
                    log_message(APP_LOG_LEVEL_INFO, "Requesting notes from menu callback");
                    request_notes(MSG_PEBBLE_REQUEST_COM_VERSION, 0);
            }
            break;
    } 
}


// WINDOW FUNCS 

void main_window_round_background_update_proc(Layer* layer, GContext *ctx){
    graphics_context_set_fill_color(ctx, COLOR_PRIMARY);
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
    app_message_open(APPMSG_INBOX_SIZE, APPMSG_OUTBOX_SIZE);

    if(!mainTimeStatusBar){
        mainTimeStatusBar = status_bar_layer_create();
    }

    //status_bar_layer_set_colors(timeStatusBar, PBL_IF_COLOR_ELSE(GColorRajah, GColorDarkGray), GColorWhite);
    #if PBL_COLOR
    status_bar_layer_set_colors(mainTimeStatusBar, COLOR_PRIMARY, COLOR_TEXT_LIGHT);
    #endif   

    //errorGraphicsLayer = layer_create(layer_get_bounds(window_layer));
    layer_add_child(window_layer, status_bar_layer_get_layer(mainTimeStatusBar));

    //generate_dummy_note_headers(5);

    if(!mainMenuLayer){
    GRect window_bounds = layer_get_bounds(window_layer);
    mainMenuLayer = menu_layer_create(GRect(0, STATUS_BAR_LAYER_HEIGHT, window_bounds.size.w, window_bounds.size.h - STATUS_BAR_LAYER_HEIGHT));
    menu_layer_set_highlight_colors(mainMenuLayer, COLOR_MENU_HIGHLIGHT, COLOR_TEXT_LIGHT);
    menu_layer_set_callbacks(mainMenuLayer, NULL, (MenuLayerCallbacks){
        .get_num_sections = menu_get_num_sections_callback,
        .get_num_rows = menu_get_num_rows_callback,
        .get_header_height = menu_get_header_height_callback,
        .draw_header = menu_draw_header_callback,
        .draw_row = menu_draw_row_callback,
        .select_click = menu_select_callback
    });
    }

    menu_layer_set_click_config_onto_window(mainMenuLayer, window);
    layer_add_child(window_layer, menu_layer_get_layer(mainMenuLayer));

    //Commented out since it doesn't currently work, and could interfere with version check'
    /*
    if(recievedNoteCount != noteCount || noteCount == 0){
        request_notes();
    }
    */

    send_note_request(MSG_PEBBLE_REQUEST_COM_VERSION, 0);

    #if DEBUG_LOAD_WINDOW
    load_window_show();
    load_window_set_percentage(0.5);
    #endif
}

//Called when removed from window stack.
void main_window_unload(Window *window){
    //layer_destroy(errorGraphicsLayer);
    //Provided workaround for crash on second call of error_window_show
    //errorGraphicsLayer = NULL;

    //Remove layers before delete.
    layer_remove_from_parent(menu_layer_get_layer(mainMenuLayer));
    layer_remove_from_parent(status_bar_layer_get_layer(mainTimeStatusBar));

    delete_note_headers();

    menu_layer_destroy(mainMenuLayer);
    status_bar_layer_destroy(mainTimeStatusBar);
}

void main_window_create(){
    //Create a new window and store it in global splashWindow
    mainWindow = window_create();

    if(!mainWindow){
        error_window_show(TEXT_ERROR_MAIN_WINDOW_CREATE_FAILED);
        return;
    }

    //Set up events (handlers = events for pebble)
    //Arguments: Window to set handlers for, set events we handle via function pointers
    window_set_window_handlers(mainWindow, (WindowHandlers){
        .load = main_window_load, 
        .unload = main_window_unload
    });

    //Generate version string
    char versionTemp[3];
    snprintf(versionTemp, sizeof(versionTemp), "%d", VERSION_MAJOR);
    strcat(versionString, versionTemp);
    strcat(versionString, ".");
    snprintf(versionTemp, sizeof(versionTemp), "%d", VERSION_MINOR);
    strcat(versionString, versionTemp);
}

void main_window_destroy(){
    //Built in window destroy function for pebble?
    window_destroy(mainWindow);
}

Window* main_window_get_window(){
    return mainWindow;
}

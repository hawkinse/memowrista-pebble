#include <pebble.h>
#include "error_window.h"
#include "load_window.h"
#include "main_window.h"
#include "note_window.h"
#include "logging.h"

Window* noteWindow;

StatusBarLayer* noteTimeStatusBar;
TextLayer* titleTextLayer;
TextLayer* bodyTextLayer;
ScrollLayer* noteScrollLayer;
Layer* noteButtonHintLayer;

ActionMenuLevel* noteMainActionMenuLevel;
ActionMenuLevel* noteEditActionMenuLevel;
ActionMenuConfig* noteActionMenuConfig;
ActionMenu* noteActionMenu;

//TODO - figure out good sizes for these arrays
NoteHeader* noteCurrentHeader;
char noteBodyText[NOTE_BODY_SIZE];
int32_t noteModifyTimestamp;

//NOTE FUNCS

void note_set_body(NoteHeader* header, char* bodyText){
    noteCurrentHeader = header;

    strncpy(noteBodyText, bodyText, sizeof(noteBodyText));

    if(titleTextLayer){
        layer_mark_dirty(text_layer_get_layer(titleTextLayer));
    }

    if(bodyTextLayer){
        layer_mark_dirty(text_layer_get_layer(bodyTextLayer));
    }
}

void note_set_modified_timestamp(int32_t timestamp){
    noteModifyTimestamp = timestamp;
}

//DICATATION FUNCS

void note_dictation_replace_title_callback(DictationSession* session, DictationSessionStatus status, char* transcription, void* ctx){
    switch(status){
        case DictationSessionStatusSuccess:
            load_window_show();
            window_stack_remove(noteWindow, false);
            send_note_edit(MSG_PEBBLE_REPLACE_TITLE, noteCurrentHeader->id, transcription);
            break;
        case DictationSessionStatusFailureTranscriptionRejected:
        case DictationSessionStatusFailureTranscriptionRejectedWithError:
            break;
        default:
            error_window_show(TEXT_ERROR_MICROPHONE);
    }

    //Needed to prevent future dictation sessions from calling this callback
    dictation_session_destroy(session);
}

void note_dictation_replace_body_callback(DictationSession* session, DictationSessionStatus status, char* transcription, void* ctx){
    switch(status){
        case DictationSessionStatusSuccess:
            load_window_show();
            window_stack_remove(noteWindow, false);
            send_note_edit(MSG_PEBBLE_REPLACE_BODY, noteCurrentHeader->id, transcription);
            break;
        case DictationSessionStatusFailureTranscriptionRejected:
        case DictationSessionStatusFailureTranscriptionRejectedWithError:
            break;
        default:
            error_window_show(TEXT_ERROR_MICROPHONE);
    }

    //Needed to prevent future dictation sessions from calling this callback
    dictation_session_destroy(session);
}

void note_dictation_append_body_callback(DictationSession* session, DictationSessionStatus status, char* transcription, void* ctx){
    switch(status){
        case DictationSessionStatusSuccess:
            load_window_show();
            window_stack_remove(noteWindow, false);
            send_note_edit(MSG_PEBBLE_APPEND_BODY, noteCurrentHeader->id, transcription);
            break;
        case DictationSessionStatusFailureTranscriptionRejected:
        case DictationSessionStatusFailureTranscriptionRejectedWithError:
            break;
        default:
            error_window_show(TEXT_ERROR_MICROPHONE);
    }

    //Needed to prevent future dictation sessions from calling this callback
    dictation_session_destroy(session);
}
//ACTION BAR FUNCS

void note_action_delete_callback(ActionMenu* menu, const ActionMenuItem* action, void* data){
    log_message(APP_LOG_LEVEL_INFO, "Delete note called");
    send_note_request(MSG_PEBBLE_DELETE_NOTE, noteCurrentHeader->id);
    load_window_show();
    window_stack_remove(noteWindow, false);
}

void note_action_replace_title_callback(ActionMenu* menu, const ActionMenuItem* action, void* data){
    log_message(APP_LOG_LEVEL_INFO, "Replace note title called");
    
    #if PBL_MICROPHONE
    log_message(APP_LOG_LEVEL_INFO, "Microphone native!");
    DictationSession *session = dictation_session_create(sizeof(noteCurrentHeader->title), note_dictation_replace_title_callback, NULL);
    if(session == NULL){
        log_message(APP_LOG_LEVEL_ERROR, "Dictation session null!");
        error_window_show(TEXT_ERROR_NULL_DICTATION);
        return;
    }
    dictation_session_start(session);
    #elif DEBUG_DUMMY_MIC
    log_message(APP_LOG_LEVEL_INFO, "Debug dummy mic - directly calling callback!");
    note_dictation_replace_title_callback(NULL, DictationSessionStatusSuccess, TEXT_DUMMY_MIC_TITLE, NULL);
    #else
    error_window_show(TEXT_ERROR_NO_MICROPHONE);
    #endif
}

void note_action_replace_body_callback(ActionMenu* menu, const ActionMenuItem* action, void* data){
    log_message(APP_LOG_LEVEL_INFO, "Replace note body called");

    #if PBL_MICROPHONE
    log_message(APP_LOG_LEVEL_INFO, "Microphone native!");
    DictationSession *session = dictation_session_create(sizeof(noteBodyText), note_dictation_replace_body_callback, NULL);
    if(session == NULL){
        log_message(APP_LOG_LEVEL_ERROR, "Dictation session null!");
        error_window_show(TEXT_ERROR_NULL_DICTATION);
        return;
    }
    dictation_session_start(session);
    #elif DEBUG_DUMMY_MIC
    log_message(APP_LOG_LEVEL_INFO, "Debug dummy mic - directly calling callback!");
    note_dictation_replace_body_callback(NULL, DictationSessionStatusSuccess, TEXT_DUMMY_MIC_BODY, NULL);
    #else
    error_window_show(TEXT_ERROR_NO_MICROPHONE);
    #endif
}

void note_action_append_body_callback(ActionMenu* menu, const ActionMenuItem* action, void* data){
    log_message(APP_LOG_LEVEL_INFO, "Append note body called");
    
    #if PBL_MICROPHONE
    log_message(APP_LOG_LEVEL_INFO, "Microphone native!");
    DictationSession *session = dictation_session_create(sizeof(noteBodyText), note_dictation_append_body_callback, NULL);
    if(session == NULL){
        log_message(APP_LOG_LEVEL_ERROR, "Dictation session null!");
        error_window_show(TEXT_ERROR_NULL_DICTATION);
        return;
    }
    dictation_session_start(session);
    #elif DEBUG_DUMMY_MIC
    log_message(APP_LOG_LEVEL_INFO, "Debug dummy mic - directly calling callback!");
    note_dictation_append_body_callback(NULL, DictationSessionStatusSuccess, TEXT_DUMMY_MIC_BODY_APPEND, NULL);    
    #else
    error_window_show(TEXT_ERROR_NO_MICROPHONE);
    #endif
}

void note_action_menu_close_pending_callback(ActionMenu* menu, const ActionMenuItem* action, void* data){
    log_message(APP_LOG_LEVEL_INFO, "Action menu close pending");
}

void note_action_menu_close_finished_callback(ActionMenu* menu, const ActionMenuItem* action, void* data){
    log_message(APP_LOG_LEVEL_INFO, "Action menu close finished");
    log_message_int(APP_LOG_LEVEL_INFO, "Free bytes: %d", heap_bytes_free());
}

// WINDOW FUNCS 
void note_button_select_callback(ClickRecognizerRef recognizer, void* context){
    log_message(APP_LOG_LEVEL_INFO, "note window middle click pressed!");
    if(noteActionMenuConfig != NULL){
        log_message_int(APP_LOG_LEVEL_INFO, "noteActionMenuConfig pointer: %p", (int)noteActionMenuConfig);
        action_menu_open(noteActionMenuConfig);
    } else {
        log_message(APP_LOG_LEVEL_ERROR, "noteActionMenuConfig is null!!!");
    }
}

void note_window_click_provider(void *context){
    window_single_click_subscribe(BUTTON_ID_SELECT, note_button_select_callback);
}

void note_window_hint_update_proc(Layer* layer, GContext* ctx){
    //Fakes the button hint shown in notifications, some apps (ex. Battery+)
    //Placeholder until I can figure out how it's officially done
    GRect layer_bounds = layer_get_bounds(layer);
    GPoint draw_point = {
        .x = layer_bounds.size.w + ((MENU_HINT_SIZE / 2) - 1),
        .y = layer_bounds.size.h / 2 + (MENU_HINT_SIZE / 2)
    };

    graphics_context_set_fill_color(ctx, COLOR_MENU_HINT);
    graphics_fill_circle(ctx, draw_point, MENU_HINT_SIZE);
}

//Called when window is placed onto window stack
void note_window_load(Window *window){
    log_message_int(APP_LOG_LEVEL_INFO, "Note window load started. Free bytes: %d", heap_bytes_free());
    Layer* window_layer = window_get_root_layer(window);

    noteTimeStatusBar = status_bar_layer_create();
    #if PBL_COLOR
    status_bar_layer_set_colors(noteTimeStatusBar, COLOR_PRIMARY, COLOR_TEXT_LIGHT);
    #endif

    layer_add_child(window_layer, status_bar_layer_get_layer(noteTimeStatusBar));
    
    noteMainActionMenuLevel = action_menu_level_create(3);

    #if PBL_MICROPHONE || DEBUG_MODE
    noteEditActionMenuLevel = action_menu_level_create(3);
    action_menu_level_add_action(noteEditActionMenuLevel, TEXT_ACTIONMENU_REPLACE_TITLE, note_action_replace_title_callback, NULL);
    action_menu_level_add_action(noteEditActionMenuLevel, TEXT_ACTIONMENU_REPLACE_BODY, note_action_replace_body_callback, NULL);
    action_menu_level_add_action(noteEditActionMenuLevel, TEXT_ACTIONMENU_APPEND_BODY, note_action_append_body_callback, NULL);
    action_menu_level_add_child(noteMainActionMenuLevel, noteEditActionMenuLevel, TEXT_ACTIONMENU_SUBMENU_EDIT);
    #endif

    action_menu_level_add_action(noteMainActionMenuLevel, TEXT_ACTIONMENU_DELETE, note_action_delete_callback, NULL);

    noteActionMenuConfig = malloc(sizeof(ActionMenuConfig));
    noteActionMenuConfig->root_level = noteMainActionMenuLevel;
    noteActionMenuConfig->will_close = note_action_menu_close_pending_callback;
    noteActionMenuConfig->did_close = note_action_menu_close_finished_callback;
    #if PBL_COLOR
    noteActionMenuConfig->colors.background = COLOR_PRIMARY;
    noteActionMenuConfig->colors.foreground = COLOR_TEXT_LIGHT;
    #endif

    GRect window_bounds = layer_get_bounds(window_layer);
    noteScrollLayer = scroll_layer_create(GRect(0, STATUS_BAR_LAYER_HEIGHT, window_bounds.size.w, window_bounds.size.h - STATUS_BAR_LAYER_HEIGHT));
    scroll_layer_set_callbacks(noteScrollLayer, (ScrollLayerCallbacks){
        .click_config_provider = (ClickConfigProvider)note_window_click_provider
    });
    scroll_layer_set_click_config_onto_window(noteScrollLayer, window);
    layer_add_child(window_layer, scroll_layer_get_layer(noteScrollLayer));

    titleTextLayer = text_layer_create(GRect(0, 0, window_bounds.size.w, 2000)); //TODO - find a way to limit this to the size of the existing content!
    text_layer_set_text(titleTextLayer, noteCurrentHeader->title);
    text_layer_set_text_alignment(titleTextLayer, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft));
    text_layer_set_font(titleTextLayer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    scroll_layer_add_child(noteScrollLayer, text_layer_get_layer(titleTextLayer));

    GSize titleSize = text_layer_get_content_size(titleTextLayer);
    bodyTextLayer = text_layer_create(GRect(0, titleSize.h + NOTE_TITLE_PADDING, window_bounds.size.w, 2000));
    text_layer_set_text(bodyTextLayer, noteBodyText);
    text_layer_set_text_alignment(bodyTextLayer, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft));
    text_layer_set_font(bodyTextLayer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    scroll_layer_add_child(noteScrollLayer, text_layer_get_layer(bodyTextLayer));

    GSize bodySize = text_layer_get_content_size(bodyTextLayer);
    scroll_layer_set_content_size(noteScrollLayer, GSize((titleSize.w > bodySize.w ? titleSize.w : bodySize.w), titleSize.h + bodySize.h + (window_bounds.size.h / 3)));

    #if PBL_ROUND
    //TODO - find a fix for cut off end text.
    scroll_layer_set_paging(noteScrollLayer, true);
    text_layer_enable_screen_text_flow_and_paging(titleTextLayer, 2);
    text_layer_enable_screen_text_flow_and_paging(bodyTextLayer, 2);
    #endif

    //Setting to full size of window is temporary. We'll eventually shrink it down to only what is needed
    noteButtonHintLayer = layer_create(layer_get_bounds(window_layer));
    layer_set_update_proc(noteButtonHintLayer, note_window_hint_update_proc);
    layer_add_child(window_layer, noteButtonHintLayer);

    log_message_int(APP_LOG_LEVEL_INFO, "Note window loaded. Free bytes: %d", heap_bytes_free());
}

//Called when removed from window stack.
void note_window_unload(Window *window){
    layer_remove_from_parent(status_bar_layer_get_layer(noteTimeStatusBar));
    layer_remove_from_parent(text_layer_get_layer(titleTextLayer));
    layer_remove_from_parent(text_layer_get_layer(bodyTextLayer));
    layer_remove_from_parent(scroll_layer_get_layer(noteScrollLayer));
    layer_remove_from_parent(noteButtonHintLayer);

    status_bar_layer_destroy(noteTimeStatusBar);
    text_layer_destroy(titleTextLayer);
    text_layer_destroy(bodyTextLayer);
    scroll_layer_destroy(noteScrollLayer);
    layer_destroy(noteButtonHintLayer);
    
    action_menu_hierarchy_destroy(noteMainActionMenuLevel, NULL, NULL);
    
    free(noteActionMenuConfig);
    noteActionMenuConfig = NULL;
    //noteCurrentHeader = NULL;

    noteTimeStatusBar = NULL;
    titleTextLayer = NULL;
    bodyTextLayer = NULL;
    noteScrollLayer = NULL;
    noteButtonHintLayer = NULL;
    noteMainActionMenuLevel = NULL;

    log_message_int(APP_LOG_LEVEL_INFO, "Note window unloaded. Free bytes: %d", heap_bytes_free());
}

void note_window_create(){
    log_message(APP_LOG_LEVEL_INFO, "Note window created!");
    //Create a new window and store it in global splashWindow
    noteWindow = window_create();

    if(!noteWindow){
        error_window_show(TEXT_ERROR_NOTE_WINDOW_CREATE_FAILED);
        return;
    }

    //Set up events (handlers = events for pebble)
    //Arguments: Window to set handlers for, set events we handle via function pointers
    window_set_window_handlers(noteWindow, (WindowHandlers){
        .load = note_window_load, 
        .unload = note_window_unload
    });
}

void note_window_destroy(){
    log_message(APP_LOG_LEVEL_INFO, "Note window destroyed!");
    //Built in window destroy function for pebble?
    window_destroy(noteWindow);
}

Window* note_window_get_window(){
    return noteWindow;
}

#pragma once

//Main window strings
#define TEXT_DUMMY_DEFAULT_TITLE "Dummy note"
#define TEXT_DUMMY_PHONE_DEFAULT_BODY "Dummy phone default body!"
#define TEXT_DUMMY_PHONE_DELETE "Debug note. Delete Ignored"
#define TEXT_ERROR_SEND_FAILED "Unable to send data to phone!"
#define TEXT_ERROR_PHONE_GENERIC "Phone side generic error"
#define TEXT_ERROR_MAIN_WINDOW_CREATE_FAILED "Unable to create main window"
#define TEXT_ERROR_UPDATE_PEBBLE "Update the pebble app!"
#define TEXT_ERROR_UPDATE_PHONE "Update the phone app!"
#define TEXT_MENU_HEADER_NOTES "Notes"
#define TEXT_MENU_HEADER_NOTES_NONE "No Notes!"
#define TEXT_MENU_HEADER_ACTIONS "Actions"
#define TEXT_MENU_HEADER_ABOUT "About"
#define TEXT_MENU_CONTENT_NEW_NOTE "New Note"
#define TEXT_MENU_CONTENT_REFRESH "Refresh Note List"
#define TEXT_MENU_CONTENT_VERSION "Version"
#define TEXT_MENU_CONTENT_DEBUG "Debug Mode"
#define TEXT_MENU_CONTENT_DEBUG_MIC "Dummy Mic"
#define TEXT_MENU_CONTENT_DEBUG_PHONE "Dummy Phone"
#define TEXT_MENU_CONTENT_DEBUG_LOGGING "Debug logging"
#define TEXT_ENABLED "Enabled"
#define TEXT_DISABLED "Disabled"
#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_COM 0

//Debug toggles
#define DEBUG_MODE 0
#define DEBUG_DUMMY_MIC 0
#define DEBUG_DUMMY_PHONE 0
#define DEBUG_LOAD_WINDOW  0

//Main window numeric constants
#define DUMMY_PHONE_DEFAULT_TIMESTAMP 0
#define DUMMY_PHONE_COM_VERSION VERSION_COM

//Set app message size to the size of a note body since that's the largest message possible
//On Aplite without dummy mic, the outbox can be a lot smaller since we don't need to send strings'
#define APPMSG_INBOX_SIZE NOTE_BODY_SIZE
#define APPMSG_OUTBOX_SIZE NOTE_BODY_SIZE
#if PBL_PLATFORM_APLITE && !(DEBUG_MODE && DEBUG_DUMMY_MIC)
#undef APPMSG_OUTBOX_SIZE
#define APPMSG_OUTBOX_SIZE 16
#endif

#define MENU_SECTION_NOTES 0
#define MENU_SECTION_ACTIONS 1
#define MENU_SECTION_ABOUT 2

#if DEBUG_MODE || PBL_MICROPHONE
#define MENU_INDEX_ACTION_NEW_NOTE 0
#define MENU_INDEX_ACTION_REFRESH 1
#else
#define MENU_INDEX_ACTION_REFRESH 0
#define MENU_INDEX_ACTION_NEW_NOTE 999
#endif

#define MENU_INDEX_ABOUT_ABOUT 0
#define MENU_INDEX_ABOUT_DEBUG 1
#define MENU_INDEX_ABOUT_DUMMY_MIC 2
#define MENU_INDEX_ABOUT_DUMMY_PHONE 3
#define MENU_INDEX_ABOUT_LOGGING 4

#define COLOR_PRIMARY PBL_IF_COLOR_ELSE(GColorBulgarianRose, GColorBlack)
#define COLOR_SECONDARY PBL_IF_COLOR_ELSE(GColorRajah, GColorDarkGray)
#define COLOR_TEXT_DARK GColorBlack
#define COLOR_TEXT_LIGHT GColorWhite
#define COLOR_MENU_HIGHLIGHT COLOR_PRIMARY

typedef enum{
    MSG_PEBBLE_REQUEST_NOTE_COUNT = 0,
    MSG_PEBBLE_REQUEST_NOTE_ID,
    MSG_PEBBLE_REQUEST_NOTE_TITLE,
    MSG_PEBBLE_REQUEST_NOTE_BODY,
    MSG_PEBBLE_REQUEST_NOTE_DATE,
    MSG_PEBBLE_REQUEST_NOTE_TIME,
    MSG_PEBBLE_DELETE_NOTE,
    MSG_PEBBLE_REPLACE_TITLE,
    MSG_PEBBLE_REPLACE_BODY,
    MSG_PEBBLE_APPEND_BODY,
    MSG_PHONE_SEND_NOTE_COUNT,
    MSG_PHONE_SEND_NOTE_ID,
    MSG_PHONE_SEND_NOTE_TITLE,
    MSG_PHONE_SEND_NOTE_BODY,
    MSG_PHONE_SEND_NOTE_DATE,
    MSG_PHONE_SEND_NOTE_TIME,
    MSG_PHONE_UPDATED,
    MSG_PHONE_GENERIC_ERROR,
    MSG_PEBBLE_NEW_NOTE,
    MSG_PEBBLE_SET_EDIT_ID,
    MSG_PEBBLE_REQUEST_COM_VERSION,
    MSG_PHONE_SEND_COM_VERSION
    
} NoteAppMessageKey;

char versionString[7];

//MESSAGE FUNCS
void send_note_request(NoteAppMessageKey msg, int32_t value);

void send_note_edit(NoteAppMessageKey msg, int32_t id, char* edit);

void request_new_note();

void request_notes();

void request_notes_if_compatible(int version);

void response_set_note_count(int32_t count);

void response_set_current_note_id(int32_t id);

void response_set_current_note_title(char* title);

void response_set_note_body(char* body);

void response_set_note_timestamp(int32_t timestamp);

//WINDOW FUNCS
void main_window_create();

void main_window_destroy();

Window *main_window_get_window();

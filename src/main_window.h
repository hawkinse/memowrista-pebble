#pragma once

#define DEBUG_MODE 1
#define DEBUG_DUMMY_MIC 1
#define DEBUG_DUMMY_PHONE 1

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
    MSG_PEBBLE_SET_EDIT_ID
} NoteAppMessageKey;

//MESSAGE FUNCS
void send_note_request(NoteAppMessageKey msg, int32_t value);

void send_note_edit(NoteAppMessageKey msg, int32_t id, char* edit);

void request_new_note();

void request_notes();

void response_set_note_count(int32_t count);

void response_set_current_note_id(int32_t id);

void response_set_current_note_title(char* title);

void response_set_note_body(char* body);

void response_set_note_timestamp(int32_t timestamp);

//WINDOW FUNCS
void main_window_create();

void main_window_destroy();

Window *main_window_get_window();
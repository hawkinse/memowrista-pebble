#pragma once

#define NOTE_BODY_SIZE 2048
#define NOTE_TITLE_PADDING 4
#define TEXT_ERROR_MICROPHONE "Microphone Error!"
#define TEXT_ERROR_NULL_DICTATION "Dictation session null!"
#define TEXT_ERROR_NO_MICROPHONE "Microphone needed!"
#define TEXT_ERROR_NOTE_WINDOW_CREATE_FAILED "Unable to create note window"
#define TEXT_DUMMY_MIC_TITLE "Dummy voice title!"
#define TEXT_DUMMY_MIC_BODY "Dummy voice body replacement!"
#define TEXT_DUMMY_MIC_BODY_APPEND "Dummy voice body appendage!"
#define TEXT_ACTIONMENU_SUBMENU_EDIT "Edit"
#define TEXT_ACTIONMENU_REPLACE_TITLE "Replace title"
#define TEXT_ACTIONMENU_REPLACE_BODY "Replace content"
#define TEXT_ACTIONMENU_APPEND_BODY "Add to content"
#define TEXT_ACTIONMENU_DELETE "Delete"

#define COLOR_MENU_HINT GColorBlack
#define MENU_HINT_SIZE 10

typedef struct{
    char title[96];
    uint32_t id;
} NoteHeader;

void note_set_body(NoteHeader* header, char* bodyText);

void note_set_modified_timestamp(int32_t timestamp);

void note_window_create();

void note_window_destroy();

Window *note_window_get_window();
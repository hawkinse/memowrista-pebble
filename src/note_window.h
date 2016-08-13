#pragma once

typedef struct{
    char title[256];
    uint32_t id;
} NoteHeader;

void note_set_body(NoteHeader* header, char* bodyText);

void note_set_modified_timestamp(int32_t timestamp);

void note_window_create();

void note_window_destroy();

Window *note_window_get_window();
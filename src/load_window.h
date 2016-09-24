#pragma once

#define COLOR_PROGRESS_BACKGROUND GColorDarkGray
#define COLOR_PROGRESS_BAR GColorWhite
#define COLOR_LOADING_TEXT COLOR_TEXT_LIGHT
#define COLOR_BACKGROUND COLOR_PRIMARY
#define PROGRESS_BAR_WIDTH 100
#define PROGRESS_BAR_HEIGHT 10
#define DISABLE_LOAD_WINDOW 0
#define IMG_LOADING RESOURCE_ID_IMAGE_LOADING_WHITE


void load_window_set_percentage(float percentage);

void load_window_show();

void load_window_create();

void load_window_destroy();

Window *load_window_get_window();
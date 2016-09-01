#pragma once

#define COLOR_PROGRESS_BACKGROUND GColorDarkGray
#define COLOR_PROGRESS_BAR GColorBlack
#define PROGRESS_BAR_WIDTH 100
#define PROGRESS_BAR_HEIGHT 10

void load_window_set_percentage(float percentage);

void load_window_show();

void load_window_create();

void load_window_destroy();

Window *load_window_get_window();
#pragma once

#define COLOR_PROGRESS_BACKGROUND PBL_IF_COLOR_ELSE(GColorDarkGray, GColorBlack)
#define COLOR_PROGRESS_BAR GColorWhite
#define COLOR_LOADING_TEXT PBL_IF_COLOR_ELSE(COLOR_TEXT_LIGHT, COLOR_TEXT_DARK)
#define COLOR_BACKGROUND PBL_IF_COLOR_ELSE(COLOR_PRIMARY, GColorDarkGray)
#define PROGRESS_BAR_WIDTH 100
#define PROGRESS_BAR_HEIGHT 10
#define DISABLE_LOAD_WINDOW 0
#define IMG_LOADING PBL_IF_COLOR_ELSE(RESOURCE_ID_IMAGE_LOADING_WHITE, RESOURCE_ID_IMAGE_LOADING_BLACK)

void load_window_set_percentage(float percentage);

void load_window_show();

void load_window_create();

void load_window_destroy();

Window *load_window_get_window();
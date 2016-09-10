#pragma once

#include <pebble.h>

#define DEBUG_LOGGING 0

inline void log_message(AppLogLevel level, char* msg){
    #if DEBUG_LOGGING
    APP_LOG(level, msg);
    #endif
}

inline void log_message_int(AppLogLevel level, char* msg, int val){
    #if DEBUG_LOGGING
    APP_LOG(level, msg, val);
    #endif
}

inline void log_message_string(AppLogLevel level, char* msg, char* val){
    #if DEBUG_LOGGING
    APP_LOG(level, msg, val);
    #endif
}

#ifndef WEBSOCKET_CLIENT_DEBUG_H
#define WEBSOCKET_CLIENT_DEBUG_H

#include <lws_config.h>
#include <libwebsockets.h>
#include <stdio.h>

// Set libwebsockets log level - uncomment for debugging
// #define LLL_ERR 1
// #define LLL_WARN 2
// #define LLL_NOTICE 3
// #define LLL_INFO 4
// #define LLL_DEBUG 5
// #define LLL_PARSER 6
// #define LLL_HEADER 7
// #define LLL_EXT 8
// #define LLL_CLIENT 9
// #define LLL_LATENCY 10
// #define LLL_COUNT 11

static void lws_log_emit_function(int level, const char *line)
{
    (void)level; // Mark level as unused to suppress warning
    fprintf(stderr, "%s\n", line);
}

static void setup_lws_logging()
{
    // Set libwebsockets debug level
    int debug_level = LLL_ERR | LLL_WARN | LLL_NOTICE;
#ifdef DEBUG
    debug_level |= LLL_INFO | LLL_DEBUG | LLL_CLIENT;
#endif
    lws_set_log_level(debug_level, lws_log_emit_function);
}

#endif // WEBSOCKET_CLIENT_DEBUG_H

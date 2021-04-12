/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
#include "config.h"
#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <memcached/extension.h>
#include <memcached/engine.h>
#include "protocol_extension.h"

static EXTENSION_LOG_LEVEL current_log_level = EXTENSION_LOG_WARNING;
SERVER_HANDLE_V1 *sapi;

static const char *get_name(void) {
    return "syslog";
}

static int map_priority(EXTENSION_LOG_LEVEL severity)
{
    int priority;
    switch (severity) {
    case EXTENSION_LOG_WARNING:
        priority = LOG_WARNING;
        break;
    case EXTENSION_LOG_INFO:
        priority = LOG_NOTICE;
        break;
    case EXTENSION_LOG_DETAIL:
        priority = LOG_INFO;
        break;
    case EXTENSION_LOG_DEBUG:
    default:
        priority = LOG_DEBUG;
    }

    return priority;
}

static void logger_log(EXTENSION_LOG_LEVEL severity,
                       const void* client_cookie,
                       const char *fmt, ...)
{
    (void)client_cookie;
    if (severity >= current_log_level) {
        char buffer[2048];
        va_list ap;
        va_start(ap, fmt);
        int len = vsnprintf(buffer, sizeof(buffer), fmt, ap);
        va_end(ap);

        if (len != -1) {
            syslog(map_priority(severity), "%s", buffer);
        }
    }
}

static EXTENSION_LOGGER_DESCRIPTOR descriptor = {
    .get_name = get_name,
    .log = logger_log
};

static void on_log_level(const void *cookie, ENGINE_EVENT_TYPE type,
                         const void *event_data, const void *cb_data) {
    if (sapi != NULL) {
        current_log_level = sapi->log->get_level();
        setlogmask(LOG_UPTO(map_priority(current_log_level)));
    }
}

MEMCACHED_PUBLIC_API
EXTENSION_ERROR_CODE memcached_extensions_initialize(const char *config,
                                                     GET_SERVER_API get_server_api) {

    sapi = get_server_api();
    if (sapi == NULL) {
        return EXTENSION_FATAL;
    }

    current_log_level = sapi->log->get_level();
    openlog("memcached", LOG_NDELAY | LOG_CONS | LOG_PID, LOG_DAEMON);
    setlogmask(LOG_UPTO(map_priority(current_log_level)));
    if (!sapi->extension->register_extension(EXTENSION_LOGGER, &descriptor)) {
        return EXTENSION_FATAL;
    }

    sapi->callback->register_callback(NULL, ON_LOG_LEVEL, on_log_level, NULL);

    return EXTENSION_SUCCESS;
}
 
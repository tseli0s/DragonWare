/**********************************************************************
 * FILE: log.h
 * PURPOSE: Kernel logging support
 * PROJECT: DragonWare Kernel
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#define LOG_MAXBUF (176)
#include <ktypes.h>
#include <macros.h>

/**
 * @brief Logging severity levels.
 * @details Used to classify log output by importance. Higher values indicate
 * more severe conditions.
 */
typedef enum {
        LOG_DEBUG   = 0, /**< Debug-level messages (verbose, for development). */
        LOG_INFO    = 1, /**< Informational messages. */
        LOG_WARNING = 2, /**< Warning messages (non-fatal issues). */
        LOG_ERROR   = 3, /**< Error messages (fatal or critical issues). */
} LogLevel;

/**
 * @brief Represents a single log message.
 * @details Stores the formatted log text in a fixed-size buffer along with an
 * optional numeric code.
 */
typedef struct _LogMessage {
        char buffer[LOG_MAXBUF]; /**< Null-terminated log message text. */
        u32  code;               /**< Optional log code (meaning defined by caller). */
} LogMessage;

/**
 * @brief Initialize the logging subsystem.
 * @details Must be called before any logging functions are used.
 */
void LogInit(void);

/**
 * @brief Log a formatted message.
 * @param level Severity level of the log.
 * @param data Format string (printf-style).
 * @param ... Format arguments.
 */
[[gnu::nonnull(2)]]
void klog(LogLevel level, const char *data, ...);

/**
 * @brief Flush any queued log messages to the display/output.
 * @details The caller is responsible for clearing the screen or output target
 * before calling this function if desired.
 */
void FlushAllLogs(void);

#ifndef debug
#ifdef DRAGONWARE_DEBUG_MODE
/* Quick and dirty debugging */
#define debug(...) klog(LOG_DEBUG, __VA_ARGS__)
#else
#define debug(...)
#endif /* DRAGONWARE_DEBUG_MODE */
#endif /* debug */

/* @brief Print a nice log message of the given level at the kernel-configured output */
#define LogMessage(level, ...) klog(level, __FILE_NAME__ ": " __VA_ARGS__)

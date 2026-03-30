#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    FOUR_LOG_DEBUG = 0,
    FOUR_LOG_INFO,
    FOUR_LOG_WARN,
    FOUR_LOG_ERROR,
} FOUR_LogLevel;

typedef struct FOUR_Logger FOUR_Logger;

/* Global setup */
void          FOUR_LogSetFile(const char* path);

/* Request a named logger (creates one if it doesn't exist yet) */
FOUR_Logger*  FOUR_GetLogger(const char* name);

/* Core log function */
void FOUR_Logger_Log(FOUR_Logger* logger, FOUR_LogLevel level, const char* fmt, ...);

/* Convenience macros */
#define FOUR_DEBUG(logger, ...) FOUR_Logger_Log((logger), FOUR_LOG_DEBUG, __VA_ARGS__)
#define FOUR_INFO(logger, ...)  FOUR_Logger_Log((logger), FOUR_LOG_INFO,  __VA_ARGS__)
#define FOUR_WARN(logger, ...)  FOUR_Logger_Log((logger), FOUR_LOG_WARN,  __VA_ARGS__)
#define FOUR_ERROR(logger, ...) FOUR_Logger_Log((logger), FOUR_LOG_ERROR, __VA_ARGS__)

/* Legacy: logs at INFO level under the "Fourtitude" logger */
void FOUR_Log(const char* fmt, ...);

#ifdef __cplusplus
}
#endif

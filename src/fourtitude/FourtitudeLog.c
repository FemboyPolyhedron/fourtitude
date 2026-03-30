#include "FourtitudeLog.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define FOUR_MAX_LOGGERS 64

struct FOUR_Logger { char name[64]; };

static FILE* s_log = NULL;
static int   s_stdout = 0;
static FOUR_Logger s_loggers[FOUR_MAX_LOGGERS];
static int s_logger_count = 0;

static const char* level_str(FOUR_LogLevel level)
{
    switch (level)
    {
        case FOUR_LOG_DEBUG: return "DEBUG";
        case FOUR_LOG_INFO: return "INFO";
        case FOUR_LOG_WARN: return "WARN";
        case FOUR_LOG_ERROR: return "ERROR";
        default: return "LOG";
    }
}

static void get_timestamp(char* buf, int size)
{
    #ifdef _WIN32
    SYSTEMTIME st;
    GetLocalTime(&st);
    snprintf(buf, size, "%02d:%02d:%02d.%03d", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    #else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm* t = localtime(&ts.tv_sec);
    snprintf(buf, size, "%02d:%02d:%02d.%03d", t->tm_hour, t->tm_min, t->tm_sec, (int)(ts.tv_nsec / 1000000));
    #endif
}


void FOUR_LogSetFile(const char* path)
{
    if (s_log) fclose(s_log);
    s_log = path ? fopen(path, "w") : NULL;
}

void FOUR_LogSetStdout(int enabled) { s_stdout = enabled; }

FOUR_Logger* FOUR_GetLogger(const char* name)
{
    if (!name || !name[0]) name = "default";

    for (int i = 0; i < s_logger_count; i++) if (strncmp(s_loggers[i].name, name, 63) == 0) return &s_loggers[i];

    if (s_logger_count >= FOUR_MAX_LOGGERS) return &s_loggers[0];  

    FOUR_Logger* l = &s_loggers[s_logger_count++];
    strncpy(l->name, name, 63);
    l->name[63] = '\0';
    return l;
}

void FOUR_Logger_Log(FOUR_Logger* logger, FOUR_LogLevel level, const char* fmt, ...)
{
    char ts[32];
    get_timestamp(ts, sizeof(ts));

    const char* name = (logger && logger->name[0]) ? logger->name : "default";

    va_list ap;
    va_start(ap, fmt);
    char body[4096];
    vsnprintf(body, sizeof(body), fmt, ap);
    va_end(ap);

    int len = (int)strlen(body);
    while (len > 0 && (body[len - 1] == '\n' || body[len - 1] == '\r')) body[--len] = '\0';

    char out[4224];
    snprintf(out, sizeof(out), "[%s] [%s/%s] %s\n", ts, name, level_str(level), body);

    if (s_stdout) { fputs(out, stdout); fflush(stdout); }

    if (s_log) { fputs(out, s_log); fflush(s_log); }
    #ifdef _WIN32
    OutputDebugStringA(out);
    #else
    fputs(out, stderr);
    #endif
}

void FOUR_Log(const char* fmt, ...)
{
    static FOUR_Logger* s_legacy = NULL;
    if (!s_legacy) s_legacy = FOUR_GetLogger("Fourtitude");

    va_list ap;
    va_start(ap, fmt);
    char buf[4096];
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    FOUR_Logger_Log(s_legacy, FOUR_LOG_INFO, "%s", buf);
}

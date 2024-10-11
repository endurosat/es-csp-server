/******************************** INCLUDE FILES *******************************/
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>

#include "es_log.h"
/*********************************** DEFINES **********************************/
#define ES_LOG_BUFFER_SIZE         8192

/*********************************** TYPEDEFS *********************************/
typedef struct _es_log_priv_t {
    log_lvl_t log_lvl;
} es_log_priv_t;
#define ES_LOG_PRIV_INIT_DEFAULT { TRACE }

/*********************************** STATE ************************************/
/* Thers is olny one context per process */
static es_log_priv_t es_log_priv = ES_LOG_PRIV_INIT_DEFAULT;

/*********************************** DATA *************************************/
static const char *log_lvl_str[LOG_LVLS] = {
    "TRACE",
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
    "CRITICAL"
};

/*************************** INTERFACE FUNCTIONS ******************************/
#define MODULE_NAME "es_log"
void
es_log_msg (const char *module, int line, int level, const char *format, ...)
{
    /* Check log lvl */
    if ((log_lvl_t)level < es_log_priv.log_lvl)
        return;

    int size = 0;
    char buffer[ES_LOG_BUFFER_SIZE];

    memset(buffer, 0x00, ES_LOG_BUFFER_SIZE);
    size = sprintf(buffer, "%s [%s] (%d): ", module, log_lvl_str[level], line);

    va_list argp;
    va_start(argp, format);
    size += vsnprintf(buffer + size, ES_LOG_BUFFER_SIZE - size, format, argp);
    va_end(argp);

    /**
     * Print to stdout
     */
    fprintf(stderr, "%s\n", buffer);

    return;
}

void es_log_set_lvl(log_lvl_t level)
{
    if (level < TRACE || level > LOG_LVLS)
    {
        int i = 0;

        ES_LOG_MSG(WARNING, "Invalid log level %d", level);
        ES_LOG_MSG(INFO, "Available log level options:");
        for (i = 0; i < LOG_LVLS; i++)
        {
            ES_LOG_MSG(INFO, "lvl: %s, priority: %d", log_lvl_str[i], i);
        }

        return;
    }
    es_log_priv.log_lvl = level;

    return;
}

log_lvl_t es_log_get_lvl (void)
{
    return es_log_priv.log_lvl;
}

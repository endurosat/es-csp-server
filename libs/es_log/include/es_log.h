/******************************************************************************
 * **File:** es_log.h
 *
 * **Description:** ES Logging module
 ******************************************************************************/

#ifndef __ES_LOG_H__
#define __ES_LOG_H__

#include <stddef.h>

/**
 * ES log levels
 *
 * Enumerators:
 *     * TRACE: A log level describing events showing step by step execution of your code that can be ignored during the standard operation, but may be useful during extended debugging sessions.
 *     * DEBUG: A log level used for events considered to be useful during software debugging when more granular information is needed.
 *     * INFO: An event happened, the event is purely informative and can be ignored during normal operations.
 *     * WARNING: Unexpected behavior happened inside the application, but it is continuing its work and the key business features are operating as expected.
 *     * ERROR: One or more functionalities are not working, preventing some functionalities from working correctly.
 *     * CRITICAL: One or more key business functionalities are not working and the whole system doesn’t fulfill the business functionalities.
 */

typedef enum _log_lvl_t
{
  TRACE = 0,
  DEBUG,
  INFO,
  WARNING,
  ERROR,
  CRITICAL,
  LOG_LVLS
} log_lvl_t;

/**
 * Macro for logging messages
 */
#define ES_LOG_MSG(level, msg, ...) es_log_msg(MODULE_NAME, __LINE__, level, msg, ## __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Set logging level
 *
 * @param[in] level Logging level to set
 */
void es_log_set_lvl (log_lvl_t level);

/**
 * Retrieve the current log level
 *
 * @return the current logging level as an integer
 */
log_lvl_t es_log_get_lvl (void);

/**
 * Log a message
 *
 * .. note:: Currently the log stream is directed to stdout but it can be redirected to any
 *  application spcefic stream.
 *
 * @param[in] module: module from which the message has been sent
 * @param[in] line: source code line number
 * @param[in] level: message level
 * @param[in] format: string formatter for the message
 */
void es_log_msg (const char *module, int line, int level, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* __ES_LOG_H__ */

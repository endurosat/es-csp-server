/******************************************************************************
 * **file:** es_tftp.h
 *
 * **Description:** ES File Transfer Protocol
 ******************************************************************************/
#ifndef __ES_TFTP_H__
#define __ES_TFTP_H__

/******************************** INCLUDE FILES *******************************/
#include <stdint.h>
#include <sys/types.h>

/********************************** TYPEDEFS **********************************/

/**
 * ES TFTP write connection type
 */
typedef ssize_t (*es_tftp_conn_write_t) (void *, uint8_t *, size_t);

/**
 * ES TFTP read connection type
 */
typedef ssize_t (*es_tftp_conn_read_t) (void *, uint8_t *, size_t);

/**
 * ES TFTP connection close type
 */
typedef int (*es_tftp_conn_close_t) (void *);

typedef struct _es_tftp_t es_tftp_t;
/*********************************** DEFINES **********************************/

/************************* INTERFACE FUNCTION PROTOTYPES **********************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Create new ES TFTP object
 */
es_tftp_t *es_tftp_new (void);

/**
 * Destroy ES TFTP object
 *
 * @param[in] self_p: Pointer to ES TFTP object reference
 */
void es_tftp_destroy (es_tftp_t **self_p);

/**
 * Register connection handlers
 *
 * @param[in] self: Reference to ES TFTP object
 * @param[in] write: Write connection handler
 * @param[in] read: Read connection handler
 * @param[in] close: Close a connection
 */
void es_tftp_register_conn_handler(es_tftp_t *self, es_tftp_conn_write_t write, es_tftp_conn_read_t read, es_tftp_conn_close_t close);

/**
 * Connection handler
 *
 * @param[in] self: Reference to ES TFTP object
 * @param[in] conn: Reference to connection
 */
void es_tftp_conn_handler(es_tftp_t *self, void *conn);

#ifdef __cplusplus
}
#endif /* _cplusplus */

#endif /* __ES_TFTP_H__ */


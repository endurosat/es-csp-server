/*****************************************************************************
 * **file:** es_file.h
 *
 * **Description:** ES File
 *****************************************************************************/

#ifndef __ES_FILE_H__
#define __ES_FILE_H__

/******************************** INCLUDE FILES *******************************/
#include <stdio.h>
#include <stdint.h>

/*********************************** TYPEDEFS *********************************/
typedef struct _es_file_t es_file_t;

/******************************* INTERFACE FUNCTIONS **************************/

/**
 * Create new ES File object
 */
es_file_t *es_file_new(const char *filename);

/**
 * Destroy ES File object.
 *
 *  @param[in] self_p: Reference to ES File object.
 */
void es_file_destroy (es_file_t **self_p);

/**
 * Retrieve the path of a file stored on the disk.
 *
 *  @param[in] self: Reference to ES File object.
 *  @return Path of the file
 */
const char * es_file_path_get(es_file_t *self);

/**
 * Check if a file exists on the disk
 *
 *  @param[in] self: Reference to ES File object.
 *  @return 0 if succesfull, -1 on error
 */
int es_file_exist(es_file_t *self);

/**
 * Remove a file from the disk
 *
 *  @param[in] self: Reference to ES File object.
 *  @return 0 if succesfull, -1 on error
 */
int es_file_remove(es_file_t *self);

/**
 * Retrieve the size of a file
 *
 *  @param[in] self: Reference to ES File object.
 *  @return 0 if succesfull, -1 on error
 */
int es_file_size_get(es_file_t *self);

/**
 * Open a file
 *
 *  @param[in] self: Reference to ES File object.
 *  @param[in] mode: Mode in which the file will be opened
 *  @return 0 if succesfull, -1 on error
 */
int es_file_open(es_file_t *self, const char *mode);

/**
 * Check if a file exists on the disk
 *
 * @param[in] self: Reference to ES File object.
 * @param[in] position: Position at which the file cursor will be set.
 * @return 0 if succesfull, -1 on error
 */
int es_file_set(es_file_t *, uint32_t position);

/**
 * Check if a file exists on the disk
 *
 * @param[in] self: Reference to ES File object.
 * @param[in] data: Buffer in which the data will be stored
 * @param[in] blksize: Size of the data block to read
 * @return 0 if succesfull, -1 on error
 */
ssize_t es_file_read(es_file_t *self, uint8_t *data, uint32_t blksize);

/**
 * Check if a file exists on the disk
 *
 * @param[in] self: Reference to ES File object.
 * @param[in] data: Data to write.
 * @param[in] blksize: Size of the data block to write.
 * @return 0 if succesfull, -1 on error
 */
ssize_t es_file_write(es_file_t *self, uint8_t *data, uint32_t blksize);

/**
 * Sync contents of the file on the disk
 *
 * @param[in] self: Reference to ES File object.
 * @return 0 if succesfull, -1 on error
 */
int es_file_sync(es_file_t *self);

/**
 * Close a file
 *
 * @param[in] self: Reference to ES File object.
 */
void es_file_close(es_file_t *self);

#endif /* __ES_FILE_H__ */


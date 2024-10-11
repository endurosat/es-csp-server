/******************************** INCLUDE FILES *******************************/
#include <unistd.h>
#include <assert.h>
#include <stdlib.h> //malloc&free
#include <string.h>

#include "es_file.h"
/******************************** LOCAL DEFINES *******************************/

#define MAX_FILE_NAME   256
/******************************* LOCAL TYPEDEFS *******************************/
struct _es_file_t {
    char filename[MAX_FILE_NAME];
    FILE *fp;
};
/********************************* LOCAL DATA *********************************/

/******************************* LOCAL FUNCTIONS ******************************/

/***************************** INTERFACE FUNCTIONS ****************************/
es_file_t *es_file_new(const char *filename)
{
    es_file_t *self = (es_file_t *)malloc(sizeof(es_file_t));
    assert(self);

    memset(self->filename, 0x00, MAX_FILE_NAME);
    strcpy(self->filename, filename);
    self->fp = NULL;

    return self;
}

void
es_file_destroy (es_file_t **self_p)
{
    assert (self_p);

    if (*self_p) {
        es_file_t *self = *self_p;
        /*
         * Free class properties here
         */

        /* If the transport layer is initialized free the object/deinit */
        if (self->fp != NULL)
            es_file_close(self);

        //  Free object itself
        free (self);

        *self_p = NULL;
    }
}

const char * es_file_path_get(es_file_t *self)
{
    return self->filename;
}

int es_file_exist(es_file_t *self)
{
    assert(self->filename);

    if (access(self->filename, F_OK) == 0)
        return 0;

    return -1;
}

int es_file_remove(es_file_t *self)
{
    return remove(self->filename);
}

int es_file_size_get(es_file_t *self)
{
    assert(self->filename);
    int len = 0;

    FILE *fp = fopen(self->filename, "r");
    if (fp == NULL)
        return -1;

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fclose(fp);

    return len;
}

int es_file_open(es_file_t *self, const char *mode)
{
    assert(self->filename);

    self->fp = fopen(self->filename, mode);
    if (self->fp == NULL)
    {
        return -1;
    }

    /* The FP should be set to unbuffered mode */
    setvbuf(self->fp, NULL, _IONBF, 0);

    return 0;
}

int es_file_set(es_file_t *self, uint32_t position)
{
    assert(self->fp);

    fseek(self->fp, position, SEEK_SET);

    return 0;
}

ssize_t es_file_read(es_file_t *self, uint8_t *data, uint32_t blksize)
{
    assert(self->fp);
    ssize_t bytes_read = 0;

    bytes_read = fread(data, 1, blksize, self->fp);

    return bytes_read;
}

ssize_t es_file_write(es_file_t *self, uint8_t *data, uint32_t blksize)
{
    assert(self->fp);
    ssize_t bytes_written = 0;

    bytes_written = fwrite(data, blksize, 1, self->fp);

    return bytes_written;
}

int es_file_sync(es_file_t *self)
{
    fflush(self->fp);
    fsync(fileno(self->fp));

    return 0;
}

void es_file_close(es_file_t *self)
{
    if (self->fp != NULL)
    {
        fclose(self->fp);
    }

    self->fp = NULL;
}

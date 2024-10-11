/******************************** INCLUDE FILES *******************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "es_file.h"
#include "es_tftp.h"
#include "es_tftp_pkt.h"

#include "es_log.h"
/******************************** LOCAL DEFINES *******************************/
#define MODULE_NAME         "es_tftp"

/********************************** TYPEDEFS **********************************/

/**
* ES TFTP
*/
struct _es_tftp_t {
    /* Connection handlers */
    es_tftp_conn_read_t read;
    es_tftp_conn_write_t write;
    es_tftp_conn_close_t close;
};

/*************************** FUNCTION PROTOTYPES ******************************/

/******************************* LOCAL DATA ***********************************/

/***************************** LOCAL FUNCTIONS ********************************/
static int es_tftp_rrq_handler(es_tftp_t *self, void *conn,
    const char *filename)
{
    int bytes_remaining = 0; /* Size of the file that remains to be sent */
    ssize_t bytes_read = 0; /* Bytes read (for a single data transaction) from the file */
    uint16_t block_number = 1; /* Block ID */
    es_file_t *file = NULL; /* File handle */

    /**
     * I/O buffers, keep i/o buffers separatelly in case we want to implement
     * retransmit.
     */
    uint8_t i_buff[ES_TFTP_PKT_MAX_SIZE];
    ssize_t i_buff_size = 0;
    es_tftp_pkt_t *i_pkt = NULL;
    uint8_t o_buff[ES_TFTP_PKT_MAX_SIZE];
    ssize_t o_buff_size = 0;
    int ret = -1;

    /* Create new file handle */
    file = es_file_new(filename);
    /* Check whether the file exists on the disk */
    if (0 != es_file_exist(file))
    {
        o_buff_size = es_tftp_opkt_err(o_buff, ES_TFTP_ERR_FILE_NOT_FOUND,
        (const uint8_t *)"File not found!");
        self->write(conn, o_buff, o_buff_size);
        goto exit;
    }

    /* Open the file for read access */
    if (0 != es_file_open(file, "rb"))
    {
        o_buff_size = es_tftp_opkt_err(o_buff, ES_TFTP_ERR_ACESS_VIOLATION,
        (const uint8_t *)"Access violation!");
        self->write(conn, o_buff, o_buff_size);
        goto exit;
    }

    /* Get the size of the file */
    bytes_remaining = es_file_size_get(file);
    /* Start sending:
     * DATA --->
     *       <--- ACK
     *    ...
     *    ...
     * DATA --->
     *       <--- ACK
     * Stop condition: bytes_read [0; 511] or intermediary step failiure
     */
    for(;;)
    {
        /**
         * 1. Create Data packet
         * 2. Send Data packet
         * 3. Receive Packet
         * 4. Check for acknowedged data block
         */

        /* 1. Create Data packet */
        o_buff_size = es_tftp_opkt_data (o_buff, block_number, es_file_read, file,
            &bytes_read);
        /**
         * We can add retransmit here.
         */
        /* 2. Send data packet */
        self->write(conn, o_buff, o_buff_size);

        /* 3. Receive Packet */
        memset(i_buff, 0xFF, ES_TFTP_PKT_MAX_SIZE);
        i_buff_size = self->read(conn, i_buff, ES_TFTP_PKT_MAX_SIZE);
        if (-1 == i_buff_size)
        {
            ES_LOG_MSG(ERROR, "Read failed!");
            break;
        }

        i_pkt = es_tftp_ipkt_get(i_buff, i_buff_size);
        if (i_pkt == NULL)
        {
            /* Mallformed packet */
            const uint8_t *err_msg = (const uint8_t *)"Mallformed TFTP packet!";
            ES_LOG_MSG(ERROR, "%s", err_msg);
            o_buff_size = es_tftp_opkt_err(o_buff, ES_TFTP_ERR_ILLEGAL_TFTP_OPERATION,
            err_msg);
            self->write(conn, o_buff, o_buff_size);
            break;
        }

        /* 4. Check for acknowedged data block */
        /* The client is telling us that an error has occured on his side */
        if (i_pkt->opcode == ES_TFTP_ERROR)
        {
            /* At this point the client has disconnected, so we close the transfer here
             * as well.
             */
            ES_LOG_MSG(ERROR, "TFTP Client error");
            break;
        }

        /**
         * Waiting for ACK of the DATA block that we have sent
         * Any other opcode is threated as an error.
         */
        if (i_pkt->opcode != ES_TFTP_ACK)
        {
            const uint8_t *err_msg = (const uint8_t *)"Illegal TFTP operation!";
            ES_LOG_MSG(ERROR, "%s", err_msg);
            o_buff_size = es_tftp_opkt_err(o_buff, ES_TFTP_ERR_ILLEGAL_TFTP_OPERATION,
            err_msg);
            self->write(conn, o_buff, o_buff_size);
            break;
        }

        /* ACK received. Check block n */
        if (i_pkt->ack.block_number != block_number)
        {
            /* TODO: Add err  */
            break;
        }

        /* Note: for larger files with size above 65535 * 512 bytes
         * The block number is rotated between 1 - 65534.
         */
        block_number = (block_number + 1) % ES_TFTP_BLOCK_N_MAX_VALUE;
        bytes_remaining -= bytes_read;
        if (!bytes_remaining && bytes_read != ES_TFTP_BLOCK_SIZE) {
            ES_LOG_MSG(TRACE, "Sent file %s size: %d", filename,
                es_file_size_get(file));
            ret = 0;
            break;
        }
    }

exit:
    es_file_destroy(&file);

    return ret;
}

static int es_tftp_wrq_handler(es_tftp_t *self, void *conn, const char *filename)
{
    uint8_t i_buff[ES_TFTP_PKT_MAX_SIZE];
    ssize_t i_buff_size = 0;
    es_tftp_pkt_t *i_pkt = NULL;
    uint8_t o_buff[ES_TFTP_PKT_MAX_SIZE];
    ssize_t o_buff_size = 0;
    es_file_t *file = NULL;
    ssize_t block_size = 0;
    int ret = -1;

    file = es_file_new(filename);
    /**
     * TBD: What should we do if the file exists ?
     * Very possible scenario if a transfer was interrupted or failed
     * for some reason.
     */
    if (0 == es_file_exist(file))
    {
        /*Truncate the file */
        es_file_remove(file);
    }

    /* Open the file for write access */
    if (0 != es_file_open(file, "wb"))
    {
        const char *err_msg = "Access violation";
        ES_LOG_MSG(ERROR, err_msg);
        o_buff_size = es_tftp_opkt_err(o_buff, ES_TFTP_ERR_ACESS_VIOLATION,
        (const uint8_t *)err_msg);
        self->write(conn, o_buff, o_buff_size);
        goto exit;
    }

    /* Start receiving:
     * ACK --->
     *       <--- DATA
     *    ...
     *    ...
     * ACK --->
     *       <--- DATA
     * ACK --->
     * Stop condition: block_size [0; 511] or intermediary step failiure
     */
    /* Send ack that we are ready to receive the file*/
    o_buff_size = es_tftp_opkt_ack(o_buff, 0);
    self->write(conn, o_buff, o_buff_size);
    for (;;)
    {
        memset(o_buff, 0xFF, ES_TFTP_PKT_MAX_SIZE);
        i_buff_size = self->read(conn, i_buff, ES_TFTP_PKT_MAX_SIZE);
        if (-1 == i_buff_size)
        {
            ES_LOG_MSG(ERROR, "Read failed!");
            break;
        }

        i_pkt = es_tftp_ipkt_get(i_buff, i_buff_size);
        if (i_pkt == NULL)
        {
            const char *err_msg = "Mallformed packet!";
            ES_LOG_MSG(ERROR, err_msg);
            o_buff_size = es_tftp_opkt_err(o_buff, ES_TFTP_ERR_ILLEGAL_TFTP_OPERATION,
                (const uint8_t *)err_msg);
            self->write(conn, o_buff, o_buff_size);
            break;
        }

        if (i_pkt->opcode == ES_TFTP_ERROR)
        {
            ES_LOG_MSG(ERROR, "Error: %s (%d)", i_pkt->err.msg, i_pkt->err.code);
            break;
        }

        /* We expect this to be a data packet */
        if (i_pkt->opcode != ES_TFTP_DATA)
        {
            const char *err_msg = "Expected data block!";
            ES_LOG_MSG(ERROR, err_msg);
            o_buff_size = es_tftp_opkt_err(o_buff, ES_TFTP_ERR_ILLEGAL_TFTP_OPERATION,
                (const uint8_t *)err_msg);
             self->write(conn, o_buff, o_buff_size);
             break;
        }

        /* Get data packet's block size */
        block_size = es_tftp_ipkt_data_size(i_pkt, i_buff_size);
        /* Write data on the disk */
        es_file_write(file, i_pkt->data.block, block_size);
        /* Send ack */
        o_buff_size = es_tftp_opkt_ack(o_buff, i_pkt->data.block_number);
        self->write(conn, o_buff, o_buff_size);
        if (block_size < ES_TFTP_BLOCK_SIZE)
        {
            /* We are done */
            ES_LOG_MSG(TRACE, "Received file %s size: %d", filename ,
                es_file_size_get(file));
            ret = 0;
            break;
        }
    }

exit:
    es_file_destroy(&file);

    return ret;
}

/**************************** INTERFACE FUNCTIONS *****************************/
es_tftp_t *es_tftp_new(void)
{
    es_tftp_t *self = NULL;
    self = (es_tftp_t *) malloc(sizeof(es_tftp_t));
    if (self == NULL)
    {
        ES_LOG_MSG(ERROR, "Memory allocation failed!");
        return NULL;
    }
    ES_LOG_MSG(DEBUG, "Creating new tftp context");

    self->write = NULL;
    self->read = NULL;
    self->close = NULL;

    return self;
}

void es_tftp_destroy(es_tftp_t **self_p)
{
    ES_LOG_MSG(DEBUG, "Destroying tftp context");
    if (*self_p)
    {
        es_tftp_t *self = *self_p;
        /*
         * Free class properties here
         */

        // Free object itself
        free (self);
        *self_p = NULL;
    }
}

void es_tftp_register_conn_handler(es_tftp_t *self, es_tftp_conn_write_t write,
    es_tftp_conn_read_t read, es_tftp_conn_close_t close)
{
    self->write = write;
    self->read = read;
    self->close = close;
}

void es_tftp_conn_handler(es_tftp_t *self, void *conn)
{
    uint8_t buff[ES_TFTP_PKT_MAX_SIZE];
    ssize_t buff_size = 0;
    es_tftp_pkt_t *i_pkt = NULL;
    /* We want to fill in the buffer with this pattern, not 0x00 in order
     * to avoid 0x00 charcters as end of strings, and differentiate mallformed
     * packets.
     */
    memset(buff, 0xFF, ES_TFTP_PKT_MAX_SIZE);

    buff_size = self->read(conn, buff, ES_TFTP_PKT_MAX_SIZE);
    if (-1 == buff_size)
    {
        ES_LOG_MSG(ERROR, "Read failed!");
        goto exit;
    }

    i_pkt = es_tftp_ipkt_get(buff, buff_size);
    if (i_pkt == NULL)
    {
        /* Mallformed packet */
        const char *err_msg = "Mallformed TFTP packet!";
        ES_LOG_MSG(ERROR, "%s", err_msg);
        buff_size = es_tftp_opkt_err(buff, ES_TFTP_ERR_ILLEGAL_TFTP_OPERATION,
            (const uint8_t *)err_msg);
        self->write(conn, buff, buff_size);
    }
    else
    {
        /**
         * Each transfer starts with RRQ or WRQ
         */
        if (ES_TFTP_RRQ == i_pkt->opcode)
        {
            ES_LOG_MSG(TRACE, "New download (RRQ) request: %s", (const char *)i_pkt->request.filename);
            /* Transfer a file from disk to client */
            es_tftp_rrq_handler(self, conn, (const char *)i_pkt->request.filename);
        }
        else if (ES_TFTP_WRQ == i_pkt->opcode)
        {
            ES_LOG_MSG(TRACE, "New upload (WRQ) request: %s", i_pkt->request.filename);
            /* Transfer a file from client to disk */
            es_tftp_wrq_handler(self, conn, (const char *)i_pkt->request.filename);
        }
        else
        {
            /**
             * Invalid opcode !WRQ || !RRQ. Send invalid opcode err
             */
            const char *err_msg = "Illegal TFTP operation!";
            buff_size = es_tftp_opkt_err(buff, ES_TFTP_ERR_ILLEGAL_TFTP_OPERATION,
            (const uint8_t *)err_msg);
            self->write(conn, buff, buff_size);
        }
    }

exit:
    self->close(conn);
    ES_LOG_MSG(INFO, "Connection closed");
}

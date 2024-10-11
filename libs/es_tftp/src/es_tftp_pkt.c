/******************************** INCLUDE FILES *******************************/
#include <stdio.h>
#include <string.h>

#include "es_tftp_pkt.h"

#include "es_log.h"
/******************************** LOCAL DEFINES *******************************/
#define MODULE_NAME  "es_tftp_pkt"

/******************************* LOCAL TYPEDEFS *******************************/

/********************************* LOCAL DATA *********************************/
const char *opcode_to_str[ES_TFTP_OPCODES] = {
    "Invalid Opcode",
    "Read Request",
    "Write Request",
    "Data Packet",
    "Acknowledge Packet",
    "Error Packet",
};

/******************************* LOCAL FUNCTIONS ******************************/

/*************************** INTERFACE FUNCTIONS ******************************/
/**
 *  Inbound Packets Processing
 */
es_tftp_pkt_t *es_tftp_ipkt_get(uint8_t *buffer, size_t buffer_len)
{
    es_tftp_pkt_t *pkt = NULL;

    /* It is not possible to receive buffer_len > ES_TFTP_PKT_MAX_SIZE
     * since we read from client connection with max length of ES_TFTP_PKT_MAX_SIZE
     */
    if (buffer_len < ES_TFTP_PKT_MIN_SIZE || buffer_len > ES_TFTP_PKT_MAX_SIZE) {
        /* Invalid packet size */
        ES_LOG_MSG(ERROR, "Invalid packet size: %d");
        return NULL;
    }

    pkt = (es_tftp_pkt_t *)buffer;
    /* Check for valid opcode */
    if (pkt->opcode < ES_TFTP_RRQ || pkt->opcode >= ES_TFTP_OPCODES)
    {
        /* Invalid opcode */
        ES_LOG_MSG(ERROR, "Invalid opcode: %d", pkt->opcode);
        return NULL;
    }

    size_t str_size = 0;
    /* Check string sizes */
    switch(pkt->opcode) {
    case ES_TFTP_RRQ:
    case ES_TFTP_WRQ:
        /* This means that we haven't received a valid string ending with '\0'
         * termination.
         */
        str_size = strlen((const char *)pkt->request.filename) + 1;
        if (str_size <= 0 || str_size > ES_TFTP_FILENAME_MAX_SIZE)
            pkt = NULL;
        break;
    case ES_TFTP_DATA:
    case ES_TFTP_ACK:
        break;
    case ES_TFTP_ERROR:
        str_size = strlen((const char *)pkt->err.msg) + 1;
        if (str_size <= 0 || str_size > ES_TFTP_ERR_MSG_MAX_SIZE)
            pkt = NULL;
        break;
    default:
        /* We shouldn't be here */
        pkt = NULL;
        break;
    }

    return pkt;
}

ssize_t es_tftp_ipkt_data_size(es_tftp_pkt_t *pkt, ssize_t buffer_len)
{
    ssize_t block_size = -1;
    if (pkt->opcode != ES_TFTP_DATA)
        return block_size;

    block_size = buffer_len - ES_TFTP_DATA_HEADER_SIZE - ES_TFTP_PKT_HEADER_SIZE;

    return block_size;
}

/**
 *  Outbound Packets Processing
 */
static ssize_t es_tftp_opkt_size(es_tftp_pkt_t *pkt, ssize_t variable_data_size)
{
    ssize_t pkt_size = -1;

    /* Add the header length */
    pkt_size = ES_TFTP_PKT_HEADER_SIZE;

    switch (pkt->opcode) {
    case ES_TFTP_RRQ:
    case ES_TFTP_WRQ:
        pkt_size += ES_TFTP_REQUEST_HEADER_SIZE;
        pkt_size += variable_data_size;
        break;
    case ES_TFTP_DATA:
        pkt_size += ES_TFTP_DATA_HEADER_SIZE;
        pkt_size += variable_data_size;
        break;
    case ES_TFTP_ACK:
        pkt_size += ES_TFTP_ACK_HEADER_SIZE;
        /* Fixed length size, no variable len field */
        break;
    case ES_TFTP_ERROR:
        pkt_size += ES_TFTP_ERR_HEADER_SIZE;
        /* + 1 for '\0' string termination */
        pkt_size += variable_data_size;
        break;
    default:
        pkt_size = -1;
        break;
    }

    /* TODO: Check for size violation */
    if (pkt_size > (ssize_t)ES_TFTP_PKT_MAX_SIZE)
        pkt_size = -1;

    return pkt_size;
}

ssize_t es_tftp_opkt_request (uint8_t *buffer, es_tftp_opcode_t opcode,
    const uint8_t *filename)
{
    ssize_t pkt_size = -1;

    es_tftp_pkt_t *pkt = (es_tftp_pkt_t *)buffer;
    pkt->opcode = opcode;
    /**
     * Note that the strlen() function doesn't count the null character \0
     * while calculating the length.
     */
    ssize_t filename_size = strlen((const char *)filename) + 1;
    if (filename_size > ES_TFTP_FILENAME_MAX_SIZE)
    {
        /* Invalid packet size */
        return pkt_size;
    }
    memcpy(pkt->request.filename, (const char *)filename, filename_size);
    pkt_size = es_tftp_opkt_size(pkt, filename_size);

    return pkt_size;
}

ssize_t es_tftp_opkt_err (uint8_t *buffer, es_tftp_err_code_t err_code,
    const uint8_t *err_msg)
{
    ssize_t pkt_size = -1;

    es_tftp_pkt_t *pkt = (es_tftp_pkt_t *)buffer;

    pkt->opcode = ES_TFTP_ERROR;
    pkt->err.code = err_code;
    /**
     * Note that the strlen() function doesn't count the null character \0
     * while calculating the length.
     */
    ssize_t err_msg_size = strlen((const char *) err_msg) + 1;
    if (err_msg_size > ES_TFTP_ERR_MSG_MAX_SIZE)
    {
        /* Invalid packet size */
        ES_LOG_MSG(ERROR, "Invalid packet size: %d", pkt_size);
        return pkt_size;
    }
    memcpy(pkt->err.msg, (const char *)err_msg, err_msg_size);
    pkt_size = es_tftp_opkt_size(pkt, err_msg_size);

    return pkt_size;
}

ssize_t es_tftp_opkt_ack (uint8_t *buffer, uint16_t block_number)
{
    ssize_t pkt_size = -1;

    es_tftp_pkt_t *pkt = (es_tftp_pkt_t *)buffer;
    pkt->opcode = ES_TFTP_ACK;
    pkt->ack.block_number = block_number;
    pkt_size = es_tftp_opkt_size(pkt, 0);

    return pkt_size;
}

ssize_t es_tftp_opkt_data (uint8_t *buffer, uint16_t block_number,
    es_tftp_pkt_data_get_t data_get, void *file_handle, ssize_t *bytes_read)
{
    ssize_t pkt_size = -1;

    es_tftp_pkt_t *pkt = (es_tftp_pkt_t *)buffer;

    pkt->opcode = ES_TFTP_DATA;
    pkt->data.block_number = block_number;
    /* Store the data */
    /**
     * NOTE: data_get handler storing the contents directly to the data.block
     * buffer avoids 1 memcpy.
     */
    *bytes_read = data_get(file_handle, pkt->data.block, ES_TFTP_BLOCK_SIZE);
    if (*bytes_read < 0)
    {
        /* TODO: err handling */
        //error
    }
    pkt_size = es_tftp_opkt_size(pkt, *bytes_read);

    return pkt_size;
}

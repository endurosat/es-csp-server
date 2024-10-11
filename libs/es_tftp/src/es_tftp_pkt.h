/*****************************************************************************
 * **file:** es_tftp_pkt.h
 *
 * **Description:** ES TFTP Packet
 *****************************************************************************/
#ifndef __ES_TFTP_PKT_H__
#define __ES_TFTP_PKT_H__

/******************************** INCLUDE FILES *******************************/
#include <stdint.h>
#include <unistd.h>

#include "es_file.h"

/*********************************** DEFINES **********************************/
/* es tftp opcode mnemonic */
typedef enum _es_tftp_opcode_t {
	 ES_TFTP_OPCODE_INVALID = 0,
	 ES_TFTP_RRQ, /**< Read request. Request a transfer of a file from the
		  server to the client */
	 ES_TFTP_WRQ, /**< Write request. Request a transfer of a file from the
		  client to the server */
	 ES_TFTP_DATA, /**< Data packet. */
	 ES_TFTP_ACK, /**< Acknowedge packet. */
	 ES_TFTP_ERROR, /**< Error packet. */
	 ES_TFTP_OPCODES
} es_tftp_opcode_t;

typedef enum _es_tftp_err_code_t {
	 ES_TFTP_ERR_NOT_DEFINED = 0,
	 ES_TFTP_ERR_FILE_NOT_FOUND, /**< The client receives this error code when the
		  requested file does not exist. */
	 ES_TFTP_ERR_ACESS_VIOLATION, /**< This error is seen when we ask for a file
		  on which we do not have the right to read or write. */
	 ES_TFTP_ERR_DISK_FULL_OR_ALLOCATION_EXCEEDED, /**<TFTP client receives this
		  error when there is limited storage area on the server. */
	 ES_TFTP_ERR_ILLEGAL_TFTP_OPERATION, /**< Any TFTP packet does not follow the RFC is
		  called illegal. A packet with an unknown opcode, a packet with a malformed
		  payload, or a packet that is out of sequence with the normal
		  flow of commands/responses would all be considered "illegal" */
	 ES_TFTP_ERR_UNKNOWN_TRANSFER_ID, /**< When a TFTP client sends a duplicate read request
		  (typically this happens when the first read request times out),
		  the requests may create an unexpected situation on the server. */
	 ES_TFTP_ERR_FILE_ALREADY_EXISTS, /** This error is received when there is a
		  file with the same name on the server. I produced this file with
		  transferring the same file to the server. */
	 ES_TFTP_ERR_NO_SUCH_USER, /**< Unused
		  Once the protocol was first adopted, it supported three modes of transferring,
		  which were netascii, octet and mail mode, which was used for sending
		  files to an email address. This error is received when the recipient
		  username does not exist on the server. This mode is not used anymore.

		  .. note:: We do not support the mode field.
		  */
} es_tftp_err_code_t;

#define ES_TFTP_BLOCK_SIZE        256
#define ES_TFTP_FILENAME_MAX_SIZE 512
#define ES_TFTP_ERR_MSG_MAX_SIZE  512
/* This is the size of the largest packet that we can send/receive */
#define ES_TFTP_PKT_MAX_SIZE (ES_TFTP_PKT_HEADER_SIZE + ES_TFTP_DATA_HEADER_SIZE + ES_TFTP_BLOCK_SIZE )
/* This is the size of the smallest packet that we can receive */
#define ES_TFTP_PKT_MIN_SIZE (ES_TFTP_PKT_HEADER_SIZE + ES_TFTP_ACK_HEADER_SIZE)
#define ES_TFTP_BLOCK_N_MAX_VALUE (65535) /* 2^8 */
/*********************************** TYPEDEFS *********************************/
typedef struct _es_tftp_request_t {
	uint8_t mode; /**< Mode (Unused) */
	uint8_t filename[]; /**< File system path. Flexible array member with size x,
							 x ∈ [1;512] */
} es_tftp_request_t;
#define ES_TFTP_REQUEST_HEADER_SIZE (sizeof(es_tftp_request_t))

typedef struct _es_tftp_data_t {
	uint16_t block_number;
	uint8_t block[]; /**< Data: Flexible array member with size x, x ∈ [1;512] */
} es_tftp_data_t;
#define ES_TFTP_DATA_HEADER_SIZE (sizeof(es_tftp_data_t))

typedef struct _es_tftp_ack_t {
	uint16_t block_number;
} es_tftp_ack_t;
#define ES_TFTP_ACK_HEADER_SIZE (sizeof(es_tftp_ack_t))

typedef struct _es_tftp_err_t {
	uint8_t code; /**< Error code. @see es_tftp_err_code_t */
	uint8_t msg[]; /**< Error mesasge in free text format.
						Flexible array member with size x, x ∈ [1;512] */
} es_tftp_err_t;
#define ES_TFTP_ERR_HEADER_SIZE (sizeof(es_tftp_err_t))

/* es tftp message structure */
typedef struct __attribute__((__packed__)) _es_tftp_pkt_t {
	uint8_t opcode; /**< Packet type @see es_tftp_opcode_t */
	union {
		es_tftp_request_t request;
		es_tftp_data_t data;
		es_tftp_ack_t ack;
		es_tftp_err_t err;
	 };
} es_tftp_pkt_t;
#define ES_TFTP_PKT_HEADER_SIZE (sizeof(uint8_t))

typedef ssize_t (*es_tftp_pkt_data_get_t) (es_file_t *file_handle, uint8_t *buffer,
	uint32_t buffer_len);
/************************** INTERFACE DATA DEFINITIONS ************************/

/****************************** INTERFACE FUNCTIONS ***************************/

/**
 * Get a new TFTP packet.
 *
 * .. note:: The packet is mapped on the memory provided by `buffer`
 *
 * @param[in] buffer: Input buffer containing TFTP packet.
 * @param[in] buffer_len: Size of the input buffer.
 * @return: Reference to ES TFTP packet (es_tftp_pkt_t)
 */
es_tftp_pkt_t *es_tftp_ipkt_get(uint8_t *buffer, size_t buffer_len);

ssize_t es_tftp_ipkt_data_size(es_tftp_pkt_t *pkt, ssize_t buffer_len);

/**
 * Create TFTP request packet
 *
 * @param[out] buffer: Storage memory for the packet
 * @param[in] opcode: Opcode RRQ or WRQ
 * @param[in] filename: Path to the file on the remote (RWQ) / local (RRQ) disk.
 * @return: Size of the packet, -1 in case of an err.
 */
ssize_t es_tftp_opkt_request (uint8_t *buffer, es_tftp_opcode_t opcode,
	 const uint8_t *filename);

/**
 * Create TFTP Error packet
 *
 * @param[out] buffer: Storage memory for the packet.
 * @param[in] err_code: Error code @see `es_tftp_err_code_t`
 * @param[in] err_msg: Error message in free text format
 * @return: Size of the packet, -1 in case of an err.
 */
ssize_t es_tftp_opkt_err (uint8_t *buffer, es_tftp_err_code_t err_code,
	const uint8_t *err_msg);

/**
 * Create TFTP Acknowledge packet
 *
 * @param[out] buffer: Storage memory for the packet.
 * @param[out] block_number: The block number of the data packet being acknowledged.
 * @return: Size of the packet, -1 in case of an err.
 */
ssize_t es_tftp_opkt_ack (uint8_t *buffer, uint16_t block_number);

/**
 * Create TFTP Data packet
 *
 * @param[out] buffer: Storage memory for the packet.
 * @param[in] block_number: Block number of the data packet.
 * @param[in] data_get: Function pointer of type `es_tftp_pkt_data_get_t`.
 * @param[in] file_handle: OS dependent file handle.
 * @param[out] bytes_read: Amount of read bytes from the file associated with
 *   `file_handle`.
 * @return: Size of the packet, -1 in case of an err.
 */
ssize_t es_tftp_opkt_data (uint8_t *buffer, uint16_t block_number,
	es_tftp_pkt_data_get_t data_get, void *file_handle, ssize_t *bytes_read);


#endif /* __ES_TFTP_PKT_H__ */

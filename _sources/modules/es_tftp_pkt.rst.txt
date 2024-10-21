ES TFTP Packet
==============

.. autocmodule:: es_tftp_pkt.h

.. contents::
    :depth: 3

Opcodes
-------

.. autocenumerator:: es_tftp_pkt.h::es_tftp_opcode_t
    :members:

Error Codes
-----------

.. autocenumerator:: es_tftp_pkt.h::es_tftp_err_code_t
    :members:

Defines
-------

.. autocmacro:: es_tftp_pkt.h::ES_TFTP_BLOCK_SIZE
.. autocmacro:: es_tftp_pkt.h::ES_TFTP_FILENAME_MAX_SIZE
.. autocmacro:: es_tftp_pkt.h::ES_TFTP_ERR_MSG_MAX_SIZE
.. autocmacro:: es_tftp_pkt.h::ES_TFTP_PKT_MAX_SIZE
.. autocmacro:: es_tftp_pkt.h::ES_TFTP_PKT_MIN_SIZE
.. autocmacro:: es_tftp_pkt.h::ES_TFTP_BLOCK_N_MAX_VALUE

Packets
-------

.. code-block:: C
   :caption: ES TFTP packet structure
   :linenos:
   :lineno-start: 1
   :name: <es_client_new>

    /* ES TFTP Packet structure */
    typedef struct __attribute__((__packed__)) _es_tftp_pkt_t {
          uint8_t opcode; /**< Packet type @see es_tftp_opcode_t */
          union {
                es_tftp_request_t request;
                es_tftp_data_t data;
                es_tftp_ack_t ack;
                es_tftp_err_t err;
           };
    } es_tftp_pkt_t;

.. autocstruct:: es_tftp_pkt.h::es_tftp_request_t
    :members:
.. autocstruct:: es_tftp_pkt.h::es_tftp_data_t
    :members:
.. autocstruct:: es_tftp_pkt.h::es_tftp_ack_t
    :members:
.. autocstruct:: es_tftp_pkt.h::es_tftp_err_t
    :members:


Interface Functions
-------------------

.. autocfunction:: es_tftp_pkt.h::es_tftp_ipkt_get
.. autocfunction:: es_tftp_pkt.h::es_tftp_ipkt_data_size
.. autocfunction:: es_tftp_pkt.h::es_tftp_opkt_request
.. autocfunction:: es_tftp_pkt.h::es_tftp_opkt_err
.. autocfunction:: es_tftp_pkt.h::es_tftp_opkt_ack
.. autocfunction:: es_tftp_pkt.h::es_tftp_opkt_data

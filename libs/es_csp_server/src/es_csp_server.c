/******************************** INCLUDE FILES *******************************/
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

/* CSP lib */
#include <csp/csp.h>
#include <csp/drivers/usart.h>
#include <csp/arch/csp_malloc.h>
#include <csp/drivers/can_socketcan.h>

#include "es_tftp.h"
#include "es_csp_server.h"

#include "es_log.h"
/******************************** LOCAL DEFINES *******************************/
#define MODULE_NAME     "csp_server"

#define ES_CSP_SERVER_READ_CON_TIMEOUT 10
#define ES_CSP_SERVER_READ_TIMEOUT 1000
#define ES_CSP_SERVER_WRITE_TIMEOUT 1000
#define ES_CSP_SERVER_BACKLOG 10

#define ES_CSP_SERVER_BUFFER_DATA_SIZE 500
#define ES_CSP_SERVER_SOCKET_OPTIONS (CSP_SO_NONE)

#define ES_CSP_TFTP_PORT     11

#define ES_CSP_ROUTER_STACK  500

#define ES_CSP_UART_IFACE_NAME "UART"
#define ES_CSP_UART_BAUDRATE 115200
#define ES_CSP_UART_DATA_BITS 8
#define ES_CSP_UART_STOP_BITS 1
#define ES_CSP_UART_PARITY_SETTINGS 0
#define ES_CSP_UART_PARITY_CHECK 0

/********************************* TYPEDEF ************************************/
typedef enum _server_csp_phy_layer_id_t {
    ES_CSP_UART = 0,
    ES_CSP_CAN,
    ES_CSP_PHY_LAYERS,
    ES_CSP_INVALID,
} server_csp_phy_layer_id_t;

/*PHY Layers struct*/
typedef struct _server_csp_phy_layers_t
{
    server_csp_phy_layer_id_t  phy_layer_id;
    const char                 *phy_layer_str;
} server_csp_phy_layers_t;

struct _es_csp_server_t
{
    /* Listening socket */
    csp_socket_t *sockfd;
    /* ES TFTP */
    es_tftp_t *es_tftp;
};

/********************************* STATE **************************************/
static volatile sig_atomic_t SERVER_STOP;

/******************************** LOCAL DATA **********************************/
static const server_csp_phy_layers_t server_csp_phy_get [ES_CSP_PHY_LAYERS] = {
    {ES_CSP_UART, "uart"},
    {ES_CSP_CAN, "can"},
};

/******************************* SIG HANDLERS *********************************/
static
void server_sig_int_handler(int sig)
{
    (void)(sig);

    SERVER_STOP = 1;

    return;
}

/******************************* LOCAL FUNCTIONS ******************************/
static server_csp_phy_layer_id_t server_csp_phy_layer_id_get(const char *str)
{
    int i = 0;

    if (str == NULL)
        return ES_CSP_INVALID;

    for (i = 0; i < ES_CSP_PHY_LAYERS; i++)
    {
        if (strcmp (str, server_csp_phy_get[i].phy_layer_str) == 0)
            return server_csp_phy_get[i].phy_layer_id;
    }

    return ES_CSP_INVALID;
}


static ssize_t server_conn_write (void *args, uint8_t *tx_buffer, size_t tx_buffer_len)
{
    int ret = CSP_ERR_NONE;
    csp_conn_t *conn = NULL;

    conn = (csp_conn_t *)args;

    csp_packet_t *packet = csp_buffer_get(ES_CSP_SERVER_BUFFER_DATA_SIZE);
    if (packet == NULL) {
        /* Could not get buffer element */
        ES_LOG_MSG(ERROR, "Failed to get CSP buffer");
        return -1;
    }
    memcpy(packet->data, tx_buffer, tx_buffer_len);
    packet->length = tx_buffer_len;
    if (1 != csp_send(conn, packet, ES_CSP_SERVER_WRITE_TIMEOUT)) {
        /* Send failed */
        ES_LOG_MSG(ERROR, "Send failed");
        csp_buffer_free(packet);
    }

    return ret;
}

static ssize_t server_conn_read (void *args, uint8_t *rx_buffer, size_t rx_buffer_len)
{
    csp_conn_t *conn = NULL;
    ssize_t bytes_read = 0;

    (void)rx_buffer_len;

    conn = (csp_conn_t *)args;

    csp_packet_t *packet = NULL;
    packet = csp_read(conn, ES_CSP_SERVER_READ_TIMEOUT);
    if (packet != NULL)
    {
        // check packet
        memcpy(rx_buffer, packet->data, packet->length);
        bytes_read = packet->length;
        csp_buffer_free(packet);
    }
    else
    {
        bytes_read = -1;
    }

    return bytes_read;
}

static int server_conn_close (void *args)
{
    int ret = 0;
    csp_conn_t *conn = NULL;
    conn = (csp_conn_t *)args;

    ret = csp_close(conn);
    if (CSP_ERR_NONE != ret)
    {
        ES_LOG_MSG(ERROR, "Failed to close connection");
    }
    conn = NULL;

    return ret;
}

/***************************** INTERFACE FUNCTIONS ****************************/
es_csp_server_t *es_csp_server_new(void)
{
    es_csp_server_t *self = NULL;

    ES_LOG_MSG(DEBUG, "Creating new server");

    self = (es_csp_server_t *)malloc(sizeof(struct _es_csp_server_t));
    if (self == NULL)
    {
        ES_LOG_MSG(ERROR, "Memory allocation failed!");
        return NULL;
    }

    self->sockfd = NULL;

    return self;
}

void es_csp_server_destroy(es_csp_server_t **self_p)
{
    ES_LOG_MSG(DEBUG, "Destroying CSP server");

    csp_free_resources();

    if (*self_p) {
        es_csp_server_t *self = *self_p;

        es_tftp_destroy(&self->es_tftp);
        /* Free object itself */
        free (self);
        *self_p = NULL;
    }
}

int es_csp_server_init(es_csp_server_t *self, int address, const char *phy_layer, const char *device)
{
    int ret = CSP_ERR_NONE;
    csp_conf_t csp_conf = {0};
    csp_iface_t *default_iface = NULL;
    server_csp_phy_layer_id_t phy_layer_id = ES_CSP_INVALID;

    phy_layer_id = server_csp_phy_layer_id_get(phy_layer);
    if (phy_layer_id == ES_CSP_INVALID)
    {
        ES_LOG_MSG(ERROR, "Invalid physical layer \"%s\"!", phy_layer);
        return -1;
    }

    /* Init CSP with address and default settings */
    csp_conf_get_defaults(&csp_conf);
    csp_conf.address = address;
    csp_conf.buffer_data_size =  ES_CSP_SERVER_BUFFER_DATA_SIZE;
    ret = csp_init(&csp_conf);
    if (ret != CSP_ERR_NONE)
    {
        ES_LOG_MSG(ERROR, "CSP init failed, error: %d", ret);
        return -1;
    }

    /* Start router task */
    csp_route_start_task(ES_CSP_ROUTER_STACK, 0);

    /* Add interface */
    if (ES_CSP_UART == phy_layer_id)
    {
        csp_usart_conf_t uart_conf;
        uart_conf.device = device;
        uart_conf.baudrate = ES_CSP_UART_BAUDRATE;
        uart_conf.databits = ES_CSP_UART_DATA_BITS;
        uart_conf.stopbits = ES_CSP_UART_STOP_BITS;
        uart_conf.paritysetting = ES_CSP_UART_PARITY_SETTINGS;
        uart_conf.checkparity = ES_CSP_UART_PARITY_CHECK;

        ret = csp_usart_open_and_add_kiss_interface(&uart_conf, ES_CSP_UART_IFACE_NAME,
                                                    &default_iface);
        if (ret != CSP_ERR_NONE)
        {
            ES_LOG_MSG(ERROR, "Failed to add serial interface interface [%s], error: %d",
                          uart_conf.device, ret);
            return -1;
        }
        ES_LOG_MSG(DEBUG, "UART iface init done");
    }
    else if (ES_CSP_CAN == phy_layer_id)
    {
        ret = csp_can_socketcan_open_and_add_interface(device, CSP_IF_CAN_DEFAULT_NAME,
            0, 0, &default_iface);
        if (ret != CSP_ERR_NONE)
        {
            ES_LOG_MSG(ERROR, "Failed to add CAN interface [%s], error: %d", device, ret);
            return -1;
        }
        ES_LOG_MSG(DEBUG, "CAN iface init done");
    }
    csp_rtable_set(CSP_DEFAULT_ROUTE, 0, default_iface, CSP_NO_VIA_ADDRESS);

    /* Create socket with no specific socket options, e.g.
       accepts CRC32, HMAC, XTEA, etc. if enabled during compilation */
    self->sockfd = csp_socket(ES_CSP_SERVER_SOCKET_OPTIONS);
    if (self->sockfd == NULL)
    {
        ES_LOG_MSG(ERROR, "Failed to create CSP socket!");
        return -1;
    }

    /* Bind socket to all ports, e.g. all incoming connections will be handled here */
    ret = csp_bind(self->sockfd, CSP_ANY);
    if (ret != CSP_ERR_NONE)
    {
        ES_LOG_MSG(ERROR, "Socket bind failed, error: %d", ret);
        csp_close(self->sockfd);
        return -1;
    }

    /* Create a backlog of 10 connections, i.e. up to 10 new connections can be queued */
    ret = csp_listen(self->sockfd, ES_CSP_SERVER_BACKLOG);
    if (ret != CSP_ERR_NONE)
    {
        ES_LOG_MSG(ERROR, "Socket listen failed, error: %d", ret);
        csp_close(self->sockfd);
        return -1;
    }

    self->es_tftp = es_tftp_new();
    es_tftp_register_conn_handler(self->es_tftp, server_conn_write,
        server_conn_read, server_conn_close);

    return 0;
}

void es_csp_server_run(es_csp_server_t *self)
{
    uint16_t dest_port = 0;
    csp_conn_t *conn = NULL;

    /* Register sig handlers */
    signal(SIGINT, server_sig_int_handler);
    signal(SIGTERM, server_sig_int_handler);

    SERVER_STOP = 0;
    ES_LOG_MSG(DEBUG, "Starting server...");

    for(;;)
    {
        if (SERVER_STOP)
            break;

        /* Wait for a new connection*/
        conn = csp_accept(self->sockfd, ES_CSP_SERVER_READ_CON_TIMEOUT);
        if (conn != NULL)
        {
            dest_port = csp_conn_dport(conn);
            ES_LOG_MSG(INFO, "New connection (destination port: %d)", dest_port);
            switch(dest_port)
            {
                case ES_CSP_TFTP_PORT:
                    es_tftp_conn_handler(self->es_tftp, (void *)conn);
                    break;
                default:
                    ES_LOG_MSG(ERROR, "Invalid service port");
                    server_conn_close ((void *)conn);
                    break;
            }
        }
    }

    csp_close(self->sockfd);

    return;
}

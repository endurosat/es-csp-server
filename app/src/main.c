/******************************** INCLUDE FILES *******************************/
#include <argp.h>
#include <stdint.h>
#include <stdlib.h>

#include "es_csp_server.h"
#include "es_log.h"
/******************************** LOCAL DEFINES *******************************/
#define MODULE_NAME     "main"

/* Default Args  */
#define ES_CSP_SERVER_ADDRESS_DFLT 10
#define ES_CSP_PHY_LAYER_DFLT "uart"
#define ES_CSP_PHY_LAYER_DEVICE_DFLT "/dev/ttyUSB0"

/******************************* LOCAL TYPEDEFS ******************************/
typedef struct _server_args_t
{
    uint8_t address;
    const char *phy_layer;
    const char *phy_device;
    int verbose;
} server_args_t;
#define ES_CSP_SERVER_DEFAULT_CFG { ES_CSP_SERVER_ADDRESS_DFLT, ES_CSP_PHY_LAYER_DFLT, \
    ES_CSP_PHY_LAYER_DEVICE_DFLT, TRACE }


/********************************* LOCAL DATA *********************************/
/* Input args table */
static struct argp_option options[] = {
    {"address", 'a', "csp address", 0, "CSP Address", 0},
    {"phy_layer", 'p', "physical layer type", 0, "Physical layer type [ uart | can ]", 0},
    {"phy_device", 'd', "device", 0, "Physical device endpoint", 0},
    {"verbose", 'v', "verbose", 0, "Set verbosity level", 0},
    { 0 }
};

/******************************* LOCAL FUNCTIONS ******************************/
static
void usage(const char *exec_name)
{
    ES_LOG_MSG(INFO, "%s -a [address] -p [physical_layer] -d [phy_device] -v [verbose level]", (char *)exec_name);
    ES_LOG_MSG(INFO, "Run '%s --help' for more information", ( char *)exec_name);

    return;
}

/*********************** INPUT ARGS PARSE CHECK FUNCS *************************/
static
error_t parse_option(int key, char *arg, struct argp_state *state)
{
     server_args_t *arguments = state->input;

    switch (key) {
        case 'a':
            arguments->address = atoi(arg);
            break;
        case 'p':
            arguments->phy_layer = arg;
            break;
        case 'd':
            arguments->phy_device = arg;
            break;
        case 'v':
            arguments->verbose = atoi(arg);
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static struct argp argp = {options, parse_option, NULL, NULL, NULL, NULL, NULL};
/********************************** MAIN **************************************/
int main(int argc, char *argv[])
{
    int ret = 0;
    server_args_t args = ES_CSP_SERVER_DEFAULT_CFG;
    es_csp_server_t *server = NULL;

    if(0 != argp_parse(&argp, argc, argv, 0, 0, &args))
    {
        usage(argv[0]);
        return -1;
    }

    es_log_set_lvl(args.verbose);

    char *init_message_fmt = "CSP addr: %d (0x%02X), device:%s (%s)";
    ES_LOG_MSG(INFO, init_message_fmt,  args.address,
          args.address, args.phy_device, args.phy_layer);

    /* Create a new server */
    server = es_csp_server_new();
    if (server == NULL)
    {
        ES_LOG_MSG(ERROR, "Failed to create CSP server!");
        return -1;
    }

    ret = es_csp_server_init(server, args.address, args.phy_layer, args.phy_device);
    if (0 == ret)
        /* The server runs until SIGTERM is sent */
        es_csp_server_run(server);
    else
        ES_LOG_MSG(ERROR, "Failed to initialize CSP server!");

    ES_LOG_MSG(INFO, "Stopping CSP server...");

    es_csp_server_destroy(&server);

    return ret;
}

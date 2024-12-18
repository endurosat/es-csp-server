/*****************************************************************************
 * **file:** server.h
 *
 * **Description:** ES CSP Server
 *****************************************************************************/
#ifndef __ES_CSP_SERVER_H__
#define __ES_CSP_SERVER_H__

/******************************** INCLUDE FILES *******************************/

/*********************************** DEFINES **********************************/

/*********************************** TYPEDEFS *********************************/

/**
 * Enable/ Disable CSP lib  debug prints
 */
typedef enum _es_csp_server_dbg_enb_t {
    CSP_DBG_DISABLED = 0, /**< Disable CSP lib debug prints */
    CSP_DBG_ENABLED, /**< Enable CSP lib debug prints */
} es_csp_server_dbg_enb_t;

/************************** INTERFACE DATA DEFINITIONS ************************/
typedef struct _es_csp_server_t es_csp_server_t;

/******************************* INTERFACE FUNCTIONS **************************/

/**
 * Create new CSP server object
 */
es_csp_server_t *es_csp_server_new(void);

/**
 * Destroy CSP server object
 *
 * @param[in] self_p: Pointer to ES CSP server object reference
 */
void es_csp_server_destroy(es_csp_server_t **self_p);

/**
 * Initialize CSP server object
 *
 * @param[in] self: Reference to ES CSP server object
 * @param[in] address: CSP server address
 * @param[in] phy_layer: Physical layer to use (`uart` or `can`)
 * @param[in] device: Device path
 * @param[in] csp_debug_enb: Enable CSP debug
 * @return 0 if succesfull, -1 on error
 */
int es_csp_server_init(es_csp_server_t *self, int address, const char *phy_layer,
    const char *device, es_csp_server_dbg_enb_t csp_debug_enb);

/**
 * Execute server main loop
 */
void es_csp_server_run(es_csp_server_t *self);

#endif /* __ES_CSP_SERVER_H__ */


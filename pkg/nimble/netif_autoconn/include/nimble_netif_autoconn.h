


#ifndef NIMBLE_NETIF_AUTOCONN_H
#define NIMBLE_NETIF_AUTOCONN_H


#ifdef __cplusplus
extern "C" {
#endif

enum {
    NIMBLE_NETIF_AUTOCONN_OK        =  0,
    NIMBLE_NETIF_AUTOCONN_STACKERR  = -1,
    NIMBLE_NETIF_AUTOCONN_ERR_ADV   = -2,
};

typedef struct {
    uint32_t period_scan;
    uint32_t period_adv;
    uint32_t period_jitter;

    uint32_t scan_itvl;
    uint32_t scan_win;

    uint32_t adv_itvl;

    uint32_t conn_scanitvl;
    uint32_t conn_scanwin;      // merge...
    uint32_t conn_itvl;
    uint16_t conn_latency;
    uint32_t conn_super_to;
    uint32_t conn_timeout;      // merge with scanitv and scanwin
    const char *name;
} nimble_netif_autoconn_cfg_t;

int nimble_netif_autoconn_init(const nimble_netif_autoconn_cfg_t *cfg);

size_t nimble_netif_autoconn_free_slots(void);

#ifdef __cplusplus
}
#endif

#endif /* NIMBLE_NETIF_AUTOCONN_H */
/** @} */

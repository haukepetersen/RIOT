


#ifndef NIMBLE_GNRC_AUTOCONN_H
#define NIMBLE_GNRC_AUTOCONN_H


#ifdef __cplusplus
extern "C" {
#endif

enum {
    AUTOCONN_OK,
    AUTOCONN_STACKERR,
};

typedef struct {
    uint32_t scan_duration;
    uint32_t scan_interval;
#ifdef USEPKG_NIMBLE
    struct ble_gap_disc_params scan_params;
#endif
    // add filter/trigger function
} autoconn_config_t;


int autoconn_run(void);

#ifdef __cplusplus
}
#endif

#endif /* NIMBLE_GNRC_AUTOCONN_H */
/** @} */

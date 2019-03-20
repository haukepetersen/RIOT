


#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TEST_MTU
#define TEST_MTU            (5000U)     /* up to 5K of payload */
#endif


/* local configuration */
#define NODE_NAME           "riot_l2cap_echo_server"
#define CONN_WAIT           (1000U)
#define SCAN_WAIT           (500U)      /* 500ms */

#define COC_CID             (0x0235)    /* random value */


/* MTU and buffer configuration */
#define MTUBUF_SPLIT        (10)        /* split in 500 byte buffers */
#define MTUBUF_NUM          (3)         /* buffer up to 3 full MTUs */

#define MBUFSIZE_OVHD       (sizeof(struct os_mbuf) + \
                             sizeof(struct os_mbuf_pkthdr))
#define MBUFSIZE_PAYLOAD    (TEST_MTU / MTUBUF_SPLIT)
#define MBUFSIZE            (MBUFSIZE_PAYLOAD + MBUFSIZE_OVHD)
#define MBUFCNT             (MTUBUF_NUM * MTUBUF_SPLIT)

void server_run(void);

void client_run(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_CONFIG_H */
/** @} */

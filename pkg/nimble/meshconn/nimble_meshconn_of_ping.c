/*
 * Copyright (C) 2021 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     pkg_nimble_meshconn_of
 * @{
 *
 * @file
 * @brief       Meshconn objective function: ICMP echo (ping)
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <errno.h>
#include <limits.h>



#include "nimble_meshconn.h"
#include "nimble_meshconn_of.h"



// static nimble_meshconn_of_t _of;



// static void _update(void)
// {
//     int of_new;

//     nimble_meshconn_nt_peer_t *peer = nimble_meshconn_nt_best(NIMBLE_MESHCONN_NT_UPSTREAM);
//     if (peer == 0) {
//         of_new = 0;
//     }
//     else {
//         of_new = peer->of + 1;
//     }

//     if (of_new != _val) {
//         _val = of_new;
//         nimble_meshconn_of_notify(of_new);
//     }
// }




// static void _on_meshconn_event(nimble_meshconn_event_t event,
//                                const uint8_t *addr)
// {
//     (void)addr;

//     DEBUG("[of_hop] mc event:%i\n", (int)event);

//     switch (event) {
//         case NIMBLE_MESHCONN_CONN_UPLINK:
//         case NIMBLE_MESHCONN_CLOSE_UPLINK:
//             _update();
//             break;
//         default:
//             /* do nothing */
//             break;
//     }
// }



int nimble_meshconn_of_ping_init(void)
{
    int res;
    (void)res;

    /* register OF */
    // res = nimble_meshconn_of_register(NIMBLE_MESHCONN_OF_PING, &_of);

    return 0;
}

# Lessons learned from faceimp and nimble_gnrc

## Assumptions
- we are the only ones using the BLE connection
    - connect means connected L2CAP channel
    - if l2cap is killed, the (GAP) connection is killed

- if `coc != NULL` -> we are connected
- if `coc == NULL` -> not connected
- but what about in between states?


## Common functionality:
- connect
- accept
- update connection parameters (should be valid for slave and master)
- terminate (valid for slave, master, advertiser)

## params:
- mbuf_pool + mbuf params (MTU size, numof)
- callbacks


### init()
- params:
    - callbacks [`connected()`, `terminated()`, data_in]

### connect()
- params:
    - ble l2 addr
    - conn_params
    - timeout
    - free context

- find a free context (context needs to be defined specifically/externally)
- (pre)init context
- call gap_connect()
- on GAP connect event:
    - [callback] on_gap_connect() -> move to L2CAP connect event
    - allocate mbuf
    - call l2cap_connect()
- on L2CAP connected event:
    - [callback] `connected()`
        - context specific action (save coc, conn_handle, etc)
    - [callback] user API event_cb(EVENT_CONNECTED)

### accept()
- params:
    - adv_params
    - free context

### terminate()
- params:
    - l2cap coc

- trigger l2cap_close()
- on L2CAP disconnected event:
    - call ble_gap_terminate()
- on GAP disconnect event:
    - [callback] `terminated()`
        - clear context
    - [callback] `user API event_cb(EVENT_TERMINATED)`


## Different but why
- pre allocation of mappings vs external handling of Imp structs



## Different by design:
### context/mapping needed

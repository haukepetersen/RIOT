# Dependency / Module System in RIOT

## Tooling needed:
- module parsing: parse `module.yml` files and build global tree
- dependency resolution: parse app and get app specific deps
- buildfile generator(s): get app context and generate Makefile/Ninja
- module verification: static test to verify module config consistency

## Feature vs. Module

Module: something that is actually build and linked
e.g.: xtimer, phydat, fmt, but also periph_gpio, cpu, and board

Feature: some arbitrary value, used for selecting modules
e.g.: cpp, arduino

Q? What about pseudomodules
Q? Can an object be both module and feature (e.g. arduino, periph_gpio)

Q? OR relationship for optional modules?!

Q? How do we model the saul_defaul construct?
Q? How can we model the inclusion of network device drivers?
Q? How to handle specific pseudomodules/features that need specific base module (e.g. at86rf233)

Q? Data structure to use for storing dependency data
- use a `set` for included modules (`usemodule`)
- use a `set` for features provided
- use a `set` for features required
- use dict for optional stuff

## Sub-modules

## Use-Cases

### A uses B
rationale:  user/module needs another module
context:    pull modules into build tree
error:      required module does not exist

examples:   uart_stdio  uses isrpipe
            isrpipe     uses tsrb

yaml:
```
uses: [B, C, D]
```

### A requires F
rationale:  module depends on given feature(s)
context:    match capabilities
error:      required features are not provided

Examples:   cpp11-compat    depends on cpp
            cc2420          depends on periph_gpio

yaml:
```
requires: [F, G, H]
```

### optional (if module A uses B if C is present)
rationale:  module uses additional modules
context:    require module only if feature|module is present

examples:   cc2420 uses gnrc_netdev in case gnrc_netdev_default is selected

yaml:
```
optional:
    - C: B
```

### provides
rationale:  module provides certain features
context:    feature are mostly provided by platforms

examples:   stm32f1     provides periph_gpio
            iotlab-m3   provides periph_spi
            arduino-due provides arduino

yaml:
```
provides: [F, G, H]
```

## Realization

- each module contains a `module.yaml` file

example - samr21-xpro:
```
---
provides:
    - periph_adc
    - periph_cpuid
    - periph_flashpage
    - periph_gpio
    - periph_i2c
    - periph_pwm
    - periph_rtc
    - periph_rtt
    - periph_spi
    - periph_timer
    - periph_uart
    - cpp

optional:
    - gnrc_netdev_default: at86rf233
    - netdev_default: at86rf233
    - saul_default: saul_gpio
```

example - cc2420:
```
---
provides: netif
uses:
    - xtimer
    - luid
    - ieee802154
    - netdev_ieee802154
    - periph_gpio
    - periph_spi
requires:
    - periph_gpio
    - periph_spi
```

example - default application:
-> pull in link layer networking (ifconfig, txtsnd) only if network devices are
   available
```
---
uses:
    - netdev_default

optional:
    - netif: [gnrc, gnrc_netdev_default, auto_init_gnrc_netif, gnrc_txtsnd, gnrc_pktdump]
```

/*
 * Copyright (C) 2013-2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    boards Boards
 * @brief       Board specific definitions and implementations
 *
 * The boards module contains all definitions and implementations, that are specific to a certain board. Boards
 * generally consist of a fixed configuration of a controller and some external devices as sensors or radios. All
 * aspects concerning pin-configurion, mcu clock and driver configuration should go into this module.
 *
 *
 * @section sec_structure File Structure
 *
 * All board implementations in RIOT follow the same file and folder structure.
 * This does not only simplify the maintenance of the growing number of boards,
 * but it also simplifies the porting of new platforms since one can simply
 * follow the existing scheme. The structure that is used is the following:
 *
 * @code{.unparsed}
 * boards/BOARD/
 *  +-- [dist/]
 *  +-- include/
 *  |    +-- board.h
 *  |    +-- periph_conf.h
 *  |    +-- [xxx_params.h]
 *  +-- board.c
 *  +-- Makefile
 *  +-- Makefile.dep
 *  +-- Makefile.include
 *  +-- Makefile.features
 * @endcode
 *
 * @note    Some (older) implementations are not quite following this structure.
 *          Though we try to keep all implementations synchronized, that might
 *          not always be the case. New implementations *SHOULD* however follow
 *          this structure, unless there are very good reasons not to...
 *
 * @subsection sub_dist dist [optional]
 *
 * This optional sub-folder holds any configuration files, scripts
 * and tools needed to flash/debug the board. Many boards hold their specific
 * OpenOCD .cfg file here.
 *
 *
 * @subsection sub_boardh include/board.h
 *
 * The board.h file bla bla bla
 *
 *
 * @subsection sub_periphconfh include/periph_conf.h
 *
 * Internal configuration
 *
 *
 * @subsection sub_xxxparamsh include/xxx_params.h [optional]
 *
 *
 * @subsection sub_boardc board.c
 *
 *
 * @subsection sub_makefile Makefile
 *
 * This file is called when actually building the board module. For most cases,
 * the file simply looks like this:
 * @code{.unparsed}
 * MODULE = board
 *
 * include $(RIOTBASE)/Makefile.base
 * @endcode
 *
 *
 * @subsection sub_makefiledep Makfile.dep
 *
 * This file is used to define the board's dependencies to other RIOT modules.
 * These are typically device drivers for on-board devices, such as sensors or
 * network adapters. All dependencies *SHOULD* only be included conditionally,
 * so the they are only compiled in if they are actively enabled by the
 * application.
 *
 * Example:
 * @code{.unparsed}
 * ifneq (,$(filter gnrc_netif_default,$(USEMODULE)))
 *   USEMODULE += at86rf231
 *   USEMODULE += gnrc_nomac
 * endif
 * @endcode
 *
 *
 * @subsection sub_makefileinclude Makefile.include
 *
 * This file contains these key configuration aspects:
 *
 * 1. the CPU that is used on the board
 * 2. the toolchain configuration to use
 * 3. configuration of flashing and debugging tools
 * 4. the configuration of the default serial port
 *
 * The **CPU** is specified through two variables: `CPU` and `CPU_MODEL`. The value
 * of `CPU` defines the CPU type, while `CPU_MODEL` should define the specific
 * CPU model. The value of `CPU` defines the path in the /cpu/ folder of the CPU
 * that is build. Please refer to @ref cpu for more information about the CPU
 * implementation.
 *
 * The **toolchain** configuration mostly depends on the CPU architecture used on
 * the board. For this reason RIOT provides pre-defined, shared Makefiles for
 * the most common toolchains and CPU architectures, e.g
. * [Makefile.include.gnu](https://github.com/RIOT-OS/RIOT/blob/master/boards/Makefile.include.gnu)
 * or [Makefile.include.cortexm_common](https://github.com/RIOT-OS/RIOT/blob/master/boards/Makefile.include.cortexm_common)
 *
 * For most boards it is sufficient, to simply include the right common Makefile
 * to configure the used toolchain, e.g.:
 * @code{.unparsed}
 * # include cortex defaults
 * include $(RIOTBOARD)/Makefile.include.cortexm_common
 * @endcode
 *
 * The configuration of the **flasher/debugger** is done in a similar manner:
 * for the most common tools RIOT already provides predefined Makefiles (e.g.
 * [Makefile.include.openocd](https://github.com/RIOT-OS/RIOT/blob/master/boards/Makefile.include.openocd)).
 *
 * Last but not least the Makefile.include needs to configure the default serial
 * port used to interact with the board (reading and writing STDIO). For most
 * situations it is sufficient to define the serial ports used by Linux and OSX
 * and include the common [Makefile.include.serial](https://github.com/RIOT-OS/RIOT/blob/master/boards/Makefile.include.serial):
 * @code{.unparsed}
 * # define the default port depending on the host OS
 * PORT_LINUX ?= /dev/ttyUSB1
 * PORT_DARWIN ?= $(shell ls -1 /dev/tty.usbserial* | head -n 1)
 *
 * # setup serial terminal
 * include $(RIOTBOARD)/Makefile.include.serial
 * @endcode
 *
 *
 * @subsection sub_makefilefeatures Makefile.features
 *
 * This file is part of the boards peripheral configuration and defines the
 * MCU peripherals that are provided (and configured) by the board. The entries
 * in this file are used by the makesystem to assess during compile time, if all
 * needed features needed by an applications are provided.
 *
 * The file *SHOUDLD* use the following template:
 * @code{.unparsed}
 * # Put defined MCU peripherals here (in alphabetical order)
 * FEATURES_PROVIDED += periph_cpuid
 * ...
 * FEATURES_PROVIDED += periph_uart
 *
 * # Various other features (if any)
 * FEATURES_PROVIDED += cpp
 *
 * # The board MPU family (used for grouping by the CI system)
 * FEATURES_MCU_GROUP = cortex_m0_1
 * @endcode
 */

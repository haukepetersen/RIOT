/*
 * Copyright (C) 2013 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    cpu CPU
 * @brief       CPU specific implementations
 *
 * This module contains all CPU specific source files. In case of multiple CPUs
 * sharing the same architecture, the implementation is split in a cpu specific
 * and a architecture specific part (eg. arm-common and lpc2387).
 *
 * @section sec_common CPU Architecture common files
 *
 *
 *
 * @section sec_structure File Structure
 *
 * All CPU implementations in RIOT *SHOULD* follow the same structure for their
 * files.
 *
 * Typical common files:
 * @code{.unparsed}
 * cpu/CPU_ARCH_common/
 *  +-- include/
 *  |    +-- cpu.h
 *  |    +-- [ARCH specific headers]
 *  +-- ldscripts/
 *  |    +-- []
 * @endcode
 *
 * @code{.unparsed}
 * cpu/CPU/
 *  +-- include/
 *  |    +-- cpu_conf.h
 *  |    +-- periph_cpu.h
 *  |    +-- VENDOR_HEADER(S)
 *  +-- ldscripts/
 *  |    +-- ...
 *  +-- periph/
 *  |    +-- cpuid.c
 *  |    +-- gpio.c
 *  |    +-- ...
 *  |    +-- uart.c
 *  |    +-- Makefile
 *  +-- cpu.c
 *  +-- Makefile
 *  +-- Makefile.dep
 *  +-- Makefile.include
 *  +-- Makefile.features
 * @endcode
 *
 */

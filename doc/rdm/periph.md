/*
 * Copyright (C) 2013-2016 Freie Universität Berlin
 * Copyright (C) 2015 INRIA
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers Drivers
 * @brief       This module contains all kind of device drivers for sensors,
 *              actuators, network devices, etc.
 *
 * As **devices** in RIOT we consider hardware, that is not part of the MCU core
 * itself and that needs an interaction with an MCU (i.e. a device driver) to
 * fulfill a meaningful function. Typical devices are sensors, actuators, and
 * network devices that are connected to the MCU via some sort of bus system
 * (e.g. SPI, I2C, plain GPIO, etc.).
 *
 *
 *
 * @section sec_model Driver model
 *
 *
 * - periph drivers @ref drivers_periph
 *
 * @subsection sub_net Network devices
 *
 * - network devices: @ref drivers_netdev_netdev2
 *
 * @subsection sub_sense Sensors
 *
 * Physical values! RAW values are no really of interest!
 *
 * - sensors/actuators: @ref drivers_saul
 *
 * @subsection sub_act Actuators
 *
 * - sensors/actuators: @ref drivers_saul
 *
 *
 * @section sec_struc Structure of drivers
 *
 * -
 *
 * Drivers must implement the following:
 *
 *
 *
 * @todo        hello
 * @todo        foobar
 *
 */

/*
 * Copyright (C) 2013-2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_periph Peripheral Driver Interface
 * @ingroup     drivers
 * @brief       The peripheral driver interface provides vendor and platform
 *              independent access to MCU peripherals.
 *
 * About
 * =====
 * One of the major features of any operating system is the abstraction of the
 * actual used hardware. In typical (IoT related) micro-controller environments,
 * we can roughly distinguish two kinds of hardware: the MCU core and
 * peripherals and external devices (e.g. sensors, actuators, network devices).
 *
 *
 * some text
 *
 * helllo1
 * -------
 * adfadsf
 *
 * hello2
 * ------
 * adsfasd
 *
 * The module contains the low-level peripheral driver interface. This interface
 * defines a standardized interface to access MCU peripherals that is not tied
 * to any specific vendor, platform or architecture.
 *
 * @todo        describe concept in detail
 * @todo        link to driver model
 * @todo        describe/link implementation guide
 * @todo        list best practices
 */




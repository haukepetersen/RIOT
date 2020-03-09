/*
 * Copyright (C) 2020 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_common_particle-mesh
 * @{
 *
 * @file
 * @brief       Name overlay for pin names used by the particle documentation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef PARTICLE_MESH_PINS_H
#define PARTICLE_MESH_PINS_H

#ifdef __cplusplus
extern "C" {
#endif

#define PARTICLE_MESH_D0            GPIO_PIN(0, 26)
#define PARTICLE_MESH_D1            GPIO_PIN(0, 27)
#define PARTICLE_MESH_D2            GPIO_PIN(1,  1)
#define PARTICLE_MESH_D3            GPIO_PIN(1,  2)
#define PARTICLE_MESH_D4            GPIO_PIN(1,  8)
#define PARTICLE_MESH_D5            GPIO_PIN(1, 10)
#define PARTICLE_MESH_D6            GPIO_PIN(1, 11)
#define PARTICLE_MESH_D7            GPIO_PIN(1, 12)
#define PARTICLE_MESH_D8            GPIO_PIN(1,  3)
#define PARTICLE_MESH_D9            GPIO_PIN(0,  6)
#define PARTICLE_MESH_D10           GPIO_PIN(0,  8)
#define PARTICLE_MESH_D11           GPIO_PIN(1, 14)
#define PARTICLE_MESH_D12           GPIO_PIN(1, 13)
#define PARTICLE_MESH_D13           GPIO_PIN(1, 15)
#define PARTICLE_MESH_D14           GPIO_PIN(0, 31)
#define PARTICLE_MESH_D15           GPIO_PIN(0, 30)
#define PARTICLE_MESH_D16           GPIO_PIN(0, 29)
#define PARTICLE_MESH_D17           GPIO_PIN(0, 28)
#define PARTICLE_MESH_D18           GPIO_PIN(0,  4)
#define PARTICLE_MESH_D19           GPIO_PIN(0,  3)
#define PARTICLE_MESH_D20           GPIO_PIN(0, 11)

#define PARTICLE_MESH_A0            PARTICLE_MESH_D19
#define PARTICLE_MESH_A1            PARTICLE_MESH_D18
#define PARTICLE_MESH_A2            PARTICLE_MESH_D17
#define PARTICLE_MESH_A3            PARTICLE_MESH_D16
#define PARTICLE_MESH_A4            PARTICLE_MESH_D15
#define PARTICLE_MESH_A5            PARTICLE_MESH_D14

#define PARTICLE_MESH_A0_LINE       ADC_LINE(1)
#define PARTICLE_MESH_A1_LINE       ADC_LINE(2)
#define PARTICLE_MESH_A2_LINE       ADC_LINE(4)
#define PARTICLE_MESH_A3_LINE       ADC_LINE(5)
#define PARTICLE_MESH_A4_LINE       ADC_LINE(6)
#define PARTICLE_MESH_A5_LINE       ADC_LINE(7)


#ifdef __cplusplus
}
#endif

#endif /* PARTICLE_MESH_PINS_H */
/** @} */

/*
 * Copyright (C) 2013 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     board_stm32f4discovery
 * @{
 *
 * @file        periph_conf.h
 * @brief       Peripheral MCU configuration for the STM32F4discovery board
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef __PERIPH_CONF_H
#define __PERIPH_CONF_H


/**
 * @brief Timer configuration
 */
#define TIMER_NUMOF         (2U)
#define TIMER_0_EN          1
#define TIMER_1_EN          1
#define TIMER_IRQ_PRIO      1

/* Timer 0 configuration */
#define TIMER_0_DEV         TIM2
#define TIMER_0_CHANNELS    4
#define TIMER_0_PRESCALER   (167U)
#define TIMER_0_MAX_VALUE   (0xffffffff)
#define TIMER_0_CLKEN()     RCC->APB1ENR |= RCC_APB1ENR_TIM2EN
#define TIMER_0_ISR         isr_tim2
#define TIMER_0_IRQ_CHAN    TIM2_IRQn

/* Timer 1 configuration */
#define TIMER_1_DEV         TIM5
#define TIMER_1_CHANNELS    4
#define TIMER_1_PRESCALER   (167U)
#define TIMER_1_MAX_VALUE   (0xffffffff)
#define TIMER_1_CLKEN()     RCC->APB1ENR |= RCC_APB1ENR_TIM5EN
#define TIMER_1_ISR         isr_tim5
#define TIMER_1_IRQ_CHAN    TIM5_IRQn


/**
 * @brief UART configuration
 */
#define UART_NUMOF          (3U)
#define UART_0_EN           1
#define UART_1_EN           1
#define UART_2_EN           1
#define UART_IRQ_PRIO       1
#define UART_CLK            (14000000U)         /* UART clock runs with 14MHz */

/* UART 0 device configuration */
#define UART_0_DEV          USART2
#define UART_0_CLKEN()      RCC->APB1ENR |= RCC_APB1ENR_USART2EN
#define UART_0_IRQ_CHAN     USART2_IRQn
#define UART_0_ISR          isr_usart2
/* UART 0 pin configuration */
#define UART_0_PORT_CLKEN() RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN
#define UART_0_PORT         GPIOA
#define UART_0_TX_PIN       2
#define UART_0_RX_PIN       3
#define UART_0_AF           7

/* UART 1 device configuration */
#define UART_1_DEV          USART1
#define UART_1_CLKEN()      RCC->APB2ENR |= RCC_APB2ENR_USART1EN
#define UART_1_IRQ_CHAN     USART1_IRQn
#define UART_1_ISR          isr_usart1
/* UART 1 pin configuration */
#define UART_1_PORT_CLKEN() RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN
#define UART_1_PORT         GPIOC
#define UART_1_TX_PIN       4
#define UART_1_RX_PIN       5
#define UART_1_AF           7

/* UART 1 device configuration */
#define UART_2_DEV          USART3
#define UART_2_CLKEN()      RCC->APB1ENR |= RCC_APB1ENR_USART3EN
#define UART_2_IRQ_CHAN     USART3_IRQn
#define UART_2_ISR          isr_usart3
/* UART 1 pin configuration */
#define UART_2_PORT_CLKEN() RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN
#define UART_2_PORT         GPIOB
#define UART_2_TX_PIN       11
#define UART_2_RX_PIN       12
#define UART_2_AF           7



/**
 * @brief ADC configuration
 */
#define ADC_NUMOF           (0U)
#define ADC_0_EN            0
#define ADC_1_EN            0

/* ADC 0 configuration */
#define ADC_0_DEV           ADC1                                                        /* TODO !!!!!!! */
#define ADC_0_SAMPLE_TIMER
/* ADC 0 channel 0 pin config */
#define ADC_0_C0_PORT
#define ADC_0_C0_PIN
#define ADC_0_C0_CLKEN()
#define ADC_0_C0_AFCFG()
/* ADC 0 channel 1 pin config */
#define ADC_0_C1_PORT
#define ADC_0_C1_PIN
#define ADC_0_C1_CLKEN()
#define ADC_0_C1_AFCFG()
/* ADC 0 channel 2 pin config */
#define ADC_0_C2_PORT
#define ADC_0_C2_PIN
#define ADC_0_C2_CLKEN()
#define ADC_0_C2_AFCFG()
/* ADC 0 channel 3 pin config */
#define ADC_0_C3_PORT
#define ADC_0_C3_PIN
#define ADC_0_C3_CLKEN()
#define ADC_0_C3_AFCFG()

/* ADC 0 configuration */
#define ADC_1_DEV           ADC2                                                        /* TODO !!!!!!! */
#define ADC_1_SAMPLE_TIMER
/* ADC 0 channel 0 pin config */
#define ADC_1_C0_PORT
#define ADC_1_C0_PIN
#define ADC_1_C0_CLKEN()
#define ADC_1_C0_AFCFG()
/* ADC 0 channel 1 pin config */
#define ADC_1_C1_PORT
#define ADC_1_C1_PIN
#define ADC_1_C1_CLKEN()
#define ADC_1_C1_AFCFG()
/* ADC 0 channel 2 pin config */
#define ADC_1_C2_PORT
#define ADC_1_C2_PIN
#define ADC_1_C2_CLKEN()
#define ADC_1_C2_AFCFG()
/* ADC 0 channel 3 pin config */
#define ADC_1_C3_PORT
#define ADC_1_C3_PIN
#define ADC_1_C3_CLKEN()
#define ADC_1_C3_AFCFG()


/**
 * @brief PWM configuration
 */
#define PWM_NUMOF           (0U)                                                        /* TODO !!!!!!! */
#define PWM_0_EN            0
#define PWM_1_EN            0

/* PWM 0 device configuration */
#define PWM_0_DEV           TIM1
#define PWM_0_CHANNELS      4
/* PWM 0 pin configuration */
#define PWM_0_PORT
#define PWM_0_PINS
#define PWM_0_PORT_CLKEN()
#define PWM_0_CH1_AFCFG()
#define PWM_0_CH2_AFCFG()
#define PWM_0_CH3_AFCFG()
#define PWM_0_CH4_AFCFG()

/* PWM 1 device configuration */
#define PWM_1_DEV           TIM3
#define PWM_1_CHANNELS      4
/* PWM 1 pin configuration */
#define PWM_1_PORT
#define PWM_1_PINS
#define PWM_1_PORT_CLKEN()
#define PWM_1_CH1_AFCFG()
#define PWM_1_CH2_AFCFG()
#define PWM_1_CH3_AFCFG()
#define PWM_1_CH4_AFCFG()



/**
 * @brief SPI configuration
 */
#define SPI_NUMOF           (0U)                                                        /* TODO !!!!!!! */
#define SPI_0_EN            0
#define SPI_1_EN            0

/* SPI 0 device config */
#define SPI_0_DEV           SPI1
#define SPI_0_CLKEN()
#define SPI_0_IRQ           SPI1_IRQn
#define SPI_0_IRQ_HANDLER
#define SPI_0_IRQ_PRIO      1
/* SPI 1 pin configuration */
#define SPI_0_PORT
#define SPI_0_PINS          ()
#define SPI_1_PORT_CLKEN()
#define SPI_1_SCK_AFCFG()
#define SPI_1_MISO_AFCFG()
#define SPI_1_MOSI_AFCFG()

/* SPI 1 device config */
#define SPI_1_DEV           SPI2
#define SPI_1_CLKEN()
#define SPI_1_IRQ           SPI2_IRQn
#define SPI_1_IRQ_HANDLER
#define SPI_1_IRQ_PRIO      1
/* SPI 1 pin configuration */
#define SPI_1_PORT
#define SPI_1_PINS          ()
#define SPI_1_PORT_CLKEN()
#define SPI_1_SCK_AFCFG()
#define SPI_1_MISO_AFCFG()
#define SPI_1_MOSI_AFCFG()


/**
 * @brief I2C configuration
 */
#define I2C_NUMOF           (0U)                                                        /* TODO !!!!!!! */
#define I2C_0_EN            0
#define I2C_0_EN            0

/* SPI 0 device configuration */
#define I2C_0_DEV           I2C1
#define I2C_0_CLKEN()
#define I2C_0_ISR           isr_i2c1
#define I2C_0_IRQ           I2C1_IRQn
#define I2C_0_IRQ_PRIO      1
/* SPI 0 pin configuration */
#define I2C_0_PORT          GPIOB
#define I2C_0_PINS          (GPIO_Pin_6 | GPIO_Pin_7)
#define I2C_0_PORT_CLKEN()
#define I2C_0_SCL_AFCFG()
#define I2C_0_SDA_AFCFG()

/* SPI 1 device configuration */
#define I2C_1_DEV           I2C2
#define I2C_1_CLKEN()
#define I2C_1_ISR           isr_i2c2
#define I2C_1_IRQ           I2C2_IRQn
#define I2C_1_IRQ_PRIO      1
/* SPI 1 pin configuration */
#define I2C_1_PORT          GPIOF
#define I2C_1_PINS          (GPIO_Pin_0 | GPIO_Pin_1)
#define I2C_1_PORT_CLKEN()
#define I2C_1_SCL_AFCFG()
#define I2C_1_SDA_AFCFG()


/**
 * @brief GPIO configuration
 */
#define GPIO_NUMOF          12
#define GPIO_0_EN           1
#define GPIO_1_EN           1
#define GPIO_2_EN           1
#define GPIO_3_EN           1
#define GPIO_4_EN           1
#define GPIO_5_EN           1
#define GPIO_6_EN           1
#define GPIO_7_EN           1
#define GPIO_8_EN           1
#define GPIO_9_EN           1
#define GPIO_10_EN          1
#define GPIO_11_EN          1
#define GPIO_IRQ_PRIO       1

/* IRQ config */
#define GPIO_IRQ_0          GPIO_0 /* alternatively GPIO_1 could be used here */
#define GPIO_IRQ_1          GPIO_2
#define GPIO_IRQ_2          GPIO_3
#define GPIO_IRQ_3          GPIO_4
#define GPIO_IRQ_4          GPIO_5
#define GPIO_IRQ_5          GPIO_6
#define GPIO_IRQ_6          GPIO_7
#define GPIO_IRQ_7          GPIO_8
#define GPIO_IRQ_8          GPIO_9
#define GPIO_IRQ_9          GPIO_10
#define GPIO_IRQ_10         GPIO_11
#define GPIO_IRQ_11         -1/* not configured */
#define GPIO_IRQ_12         -1/* not configured */
#define GPIO_IRQ_13         -1/* not configured */
#define GPIO_IRQ_14         -1/* not configured */
#define GPIO_IRQ_15         -1/* not configured */

/* GPIO channel 0 config */
#define GPIO_0_PORT         GPIOA                   /* Used for user button 1 */
#define GPIO_0_PIN          0
#define GPIO_0_CLKEN()      RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN
#define GPIO_0_EXTI_MAP()   SYSCFG->EXTICR[0] &= ~(0xf <<0); SYSCFG->EXTICR[0] |= (0 << 0);
/* GPIO channel 1 config */
#define GPIO_1_PORT         GPIOE                   /* LIS302DL INT1 */
#define GPIO_1_PIN          0
#define GPIO_1_CLKEN()      RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN
#define GPIO_1_EXTI_MAP()   SYSCFG->EXTICR[0] &= ~(0xf <<0); SYSCFG->EXTICR[0] |= (4 << 0);
/* GPIO channel 2 config */
#define GPIO_2_PORT         GPIOE                   /* LIS302DL INT2 */
#define GPIO_2_PIN          1
#define GPIO_2_CLKEN()      RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN
#define GPIO_2_EXTI_MAP()   SYSCFG->EXTICR[0] &= ~(0xf <<4); SYSCFG->EXTICR[0] |= (4 << 4);
/* GPIO channel 3 config */
#define GPIO_3_PORT         GPIOE
#define GPIO_3_PIN          2
#define GPIO_3_CLKEN()      RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN
#define GPIO_3_EXTI_MAP()   SYSCFG->EXTICR[0] &= ~(0xf <<8); SYSCFG->EXTICR[0] |= (4 << 8);
/* GPIO channel 4 config */
#define GPIO_4_PORT         GPIOE                   /* LIS302DL CS */
#define GPIO_4_PIN          3
#define GPIO_4_CLKEN()      RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN
#define GPIO_4_EXTI_MAP()   SYSCFG->EXTICR[0] &= ~(0xf <<12); SYSCFG->EXTICR[0] |= (4 << 12);
/* GPIO channel 5 config */
#define GPIO_5_PORT         GPIOD                   /* CS43L22 RESET */
#define GPIO_5_PIN          4
#define GPIO_5_CLKEN()      RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN
#define GPIO_5_EXTI_MAP()   SYSCFG->EXTICR[1] &= ~(0xf <<0); SYSCFG->EXTICR[1] |= (3 << 0);
/* GPIO channel 6 config */
#define GPIO_6_PORT         GPIOD                   /* LD8 */
#define GPIO_6_PIN          5
#define GPIO_6_CLKEN()      RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN
#define GPIO_6_EXTI_MAP()   SYSCFG->EXTICR[1] &= ~(0xf <<4); SYSCFG->EXTICR[1] |= (3 << 4);
/* GPIO channel 7 config */
#define GPIO_7_PORT         GPIOD
#define GPIO_7_PIN          6
#define GPIO_7_CLKEN()      RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN
#define GPIO_7_EXTI_MAP()   SYSCFG->EXTICR[1] &= ~(0xf <<8); SYSCFG->EXTICR[1] |= (3 << 8);
/* GPIO channel 8 config */
#define GPIO_8_PORT         GPIOD
#define GPIO_8_PIN          7
#define GPIO_8_CLKEN()      RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN
#define GPIO_8_EXTI_MAP()   SYSCFG->EXTICR[1] &= ~(0xf <<12); SYSCFG->EXTICR[1] |= (3 << 12);
/* GPIO channel 9 config */
#define GPIO_9_PORT         GPIOD
#define GPIO_9_PIN          8
#define GPIO_9_CLKEN()      RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN
#define GPIO_9_EXTI_MAP()   SYSCFG->EXTICR[2] &= ~(0xf <<0); SYSCFG->EXTICR[2] |= (3 << 0);
/* GPIO channel 10 config */
#define GPIO_10_PORT        GPIOA                   /* LD7 */
#define GPIO_10_PIN         9
#define GPIO_10_CLKEN()     RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN
#define GPIO_10_EXTI_MAP()  SYSCFG->EXTICR[2] &= ~(0xf <<4); SYSCFG->EXTICR[2] |= (0 << 4);
/* GPIO channel 11 config */
#define GPIO_11_PORT        GPIOD
#define GPIO_11_PIN         10
#define GPIO_11_CLKEN()     RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN
#define GPIO_11_EXTI_MAP()  SYSCFG->EXTICR[2] &= ~(0xf <<8); SYSCFG->EXTICR[2] |= (3 << 8);


#endif /* __PERIPH_CONF_H */

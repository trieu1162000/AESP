/*
 * switch.h
 *
 *  Created on: Apr 9, 2024
 *      Author: Trieu Huynh Pham Nhat
 */

#ifndef SWITCHES_H_
#define SWITCHES_H_

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#define TARGET_IS_BLIZZARD_RB1
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"

#define SW_GPIO_PERIPH SYSCTL_PERIPH_GPIOF
#define SW_GPIO_BASE GPIO_PORTF_BASE
#define SW1_PIN GPIO_PIN_4

typedef enum
{
    PRESSED,
    RELEASED
} sw_t;

extern void switchInit(void);
extern sw_t switchState(void);

#endif /* SWITCHES_H_ */

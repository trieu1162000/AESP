/*
 * led.h
 *
 *  Created on: Apr 9, 2024
 *      Author: Trieu Huynh Pham Nhat
 */

#ifndef LED_H_
#define LED_H_
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#define TARGET_IS_BLIZZARD_RB1
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"

#define LED_GPIO_PERIPH         (SYSCTL_PERIPH_GPIOF)
#define LED_GPIO_BASE           (GPIO_PORTF_BASE)

enum ledNumber
{
    LEDRED = 0,     // Alarm LED
    LEDGREEN,       // Door
};
enum ledValue
{
    OFF = 0,
    ON = 1,
};

extern void ledInit(void);
extern void ledControl(enum ledNumber, enum controlState);

#endif /* LED_H_ */

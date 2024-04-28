/*
 * led.c
 *
 *  Created on: Apr 9, 2024
 *      Author: Trieu Huynh Pham Nhat
 */
#include "led.h"

const uint32_t ledPin[3] = {GPIO_INT_PIN_1, GPIO_INT_PIN_2, GPIO_INT_PIN_3};

void ledInit(void)
{
    SysCtlPeripheralEnable(LED_GPIO_PERIPH);
    GPIOPinTypeGPIOOutput(LED_GPIO_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
}

void ledControl(enum ledNumber led, enum ledValue val)
{
    if (val)
        GPIOPinWrite(LED_GPIO_BASE, ledPin[led], ledPin[led]);
    else 
        GPIOPinWrite(LED_GPIO_BASE, ledPin[led], 0);
}

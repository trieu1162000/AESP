/*
 * led.c
 *
 *  Created on: Apr 9, 2024
 *      Author: Trieu Huynh Pham Nhat
 */
#include "led.h"

const uint32_t ledSetVal[3] = {1 << 1, 1 << 2, 1 << 3};
const uint32_t ledPin[3] = {GPIO_INT_PIN_1, GPIO_INT_PIN_2, GPIO_INT_PIN_3};

uint8_t led_val = 0U;

void ledTimerIntHandler(void)
{
    // Clear the timer interrupt
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    // Trigger the led val

}

void ledInit(void)
{
    SysCtlPeripheralEnable(LED_GPIO_PERIPH);
    GPIOPinTypeGPIOOutput(LED_GPIO_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
}

void ledControl(uint8_t val)
{
    // The timer will trigger the LED. Only need to disable/enable the timer
    // TODO: Enable the timer and write to the led
    if (val == 1)

    // TODO: Disable the timer
    else
}

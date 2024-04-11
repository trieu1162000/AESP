/*
 * leds_buzzer_lock.c
 *
 *  Created on: Nov 25, 2023
 *      Author: trieu
 */

#include "../inc/leds_buzzer_lock.h"

const uint32_t ledPin[2] = {
                            GPIO_PIN_5, // PA5 - LED Green
                            GPIO_PIN_4  // PB4 - LED Red
                            };
const uint32_t ledPort[2] = {
                            GPIO_PORTA_BASE, // PA5 - LED Green
                            GPIO_PORTB_BASE  // PB4 - LED Red
                            };
const uint32_t buzzerPin = GPIO_PIN_4; // PE4
const uint32_t lockPin = GPIO_PIN_5;   // PE5

void ledControl(enum ledNumber led, enum controlState state)
{
    if (state)
        GPIOPinWrite(ledPort[led], ledPin[led], ledPin[led]);
    else 
        GPIOPinWrite(ledPort[led], ledPin[led], 0);
}

void buzzerControl(enum controlState state)
{
    if (state)
        GPIOPinWrite(GPIO_PORTE_BASE, buzzerPin, buzzerPin);
    else 
        GPIOPinWrite(GPIO_PORTE_BASE, buzzerPin, 0);
}

void lockControl(enum controlState state)
{
    if (state)
        GPIOPinWrite(GPIO_PORTE_BASE, lockPin, lockPin);
    else
        GPIOPinWrite(GPIO_PORTE_BASE, lockPin, 0);
}



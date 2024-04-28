/*
 * ledControlStateMachine.c
 *
 *  Created on: Apr 9, 2024
 *      Author: trieu
 */

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "led.h"
#include "ledControlStateMachine.h"
#include "motionDetectorStateMachine.h"

enum ledValue led_val = OFF;
uint32_t ledTimer = 500U;

static ledControlState_t ledState = S_LEDOFF;

static void alarmLEDControl(enum ledValue val)
{
    ledControl(LEDRED, val);
}

void ledControlStateMachineUpdate(void)
{
    switch (ledState)
    {
    case S_LEDOFF:
        if (getMotionSensorValue() == 1)
        {
            ledState = S_LEDON;
        }
        break;
    case S_LEDON:
        if (getMotionSensorValue() == 0)
        {
            ledState = S_LEDOFF;
        }
        break;
    }

    switch (ledState)
    {
    case S_LEDOFF:
        // Turn off the LED
        alarmLEDControl(OFF);
        break;
    case S_LEDON:
        // Turn on the LED with 1 Hz frequency
        alarmLEDControl(led_val);
    }
}

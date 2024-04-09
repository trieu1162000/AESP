/*
 * ledControlStateMachine.c
 *
 *  Created on: Apr 9, 2024
 *      Author: trieu
 */

#include "motionDetectorMachine.h"
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "switches.h"
#include "led.h"
#include "utils/uartstdio.h"
#include "debug.h"

static ledControlState_t ledState = S_LEDOFF;

#ifdef DEBUG
static const char *stateName[4] = {
    "S_LEDOFF",
    "S_LEDON",
};

#endif
void ledControlStateMachineUpdate(void)
{
    switch (ledState)
    {
    case S_LEDOFF:
        if (getMotionSensorValue() == 1)
        {
            ledState = S_LEDON;
            DBG("ledState = %s\n", stateName[ledState]);
        }
        break;
    case S_LEDON:
        if (getMotionSensorValue() == 0)
        {
            ledState = S_LEDOFF;
            DBG("ledState = %s\n", stateName[ledState]);
        }
        break;
    }

    switch (ledState)
    {
    case S_LEDOFF:
        // TODO: Disable the timer

        // Turn off the LED
        led_val = 0U;
        ledControl(led_val)
        break;
    case S_MOTION:
        // TODO: Enable the timer

        // Turn on the LED with 1 Hz frequency
        led_val = 1U;
        ledControl(led_val)
    }
}

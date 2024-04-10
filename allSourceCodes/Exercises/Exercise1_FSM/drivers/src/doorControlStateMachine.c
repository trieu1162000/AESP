/*
 * doorControlStateMachine.c
 *
 *  Created on: Apr 9, 2024
 *      Author: Trieu Huynh Pham Nhat
 */
#include "doorControlStateMachine.h"
#include "led.h"

uint32_t doorTimer = 0U;

static doorState_t doorState = S_CLOSE;

#ifdef DEBUG
static const char *stateName[3] = {
    "S_CLOSE",
    "S_OPEN",
    "S_OPEN_WAIT",
};

#endif

static void doorControl(enum doorValue val)
{
    // Green LED
    ledControl()
}

void doorControlStateMachineUpdate(void)
{
    switch (doorState)
    {
    case S_CLOSE:
        if (getMotionSensorValue() == 1)
        {
            doorState = S_OPEN;
            DBG("doorState = %s\n", stateName[doorState]);
        }
        break;
    case S_OPEN:
        if (getMotionSensorValue() == 0)
        {
            doorState = S_OPEN_WAIT;
            // Set the timer to 10 s
            doorTimer = 10000;
            DBG("doorState = %s\n", stateName[doorState]);
        }
        break;
    case S_OPEN_WAIT:
        if (getMotionSensorValue() == 1)
        {
            doorState = S_OPEN;
            DBG("doorState = %s\n", stateName[doorState]);
        }
        else
        {
            if (doorTimer == 0U)
            {
                doorState = S_CLOSE;
                DBG("doorState = %s\n", stateName[doorState]);
            }
        }
        break;
    }

    switch (doorState)
    {
    case S_CLOSE:
        // Close the door
        doorControl(CLOSE);
        break;
    case S_OPEN:
    case S_OPEN_WAIT:
        // Open the door
        doorControl(OPEN);
    }
}

/*
 * systemTask.c
 *
 *  Created on: Jan 28, 2024
 *      Author: trieu
 */

#include "system_task.h"
#include <task.h>
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"


void SW1Task(void *pvParameters)
{
    // Enable interrupts on these pins here
    GPIOIntEnable(KEYPAD_PORT_BASE, KEYPAD_ROWS);

    for (;;)
    {

        // Block until a UART trigger is received...
        BaseType_t taken = xSemaphoreTake(buttonEventSemaphore_, portMAX_DELAY);
        if (pdFAIL == taken)
        {
            continue;
        }

        xSemaphoreGive(dispatchEventSemaphore_);
    }
}

void SW2Task(void *pvParameters)
{
    // Enable interrupts on these pins here
    GPIOIntEnable(KEYPAD_PORT_BASE, KEYPAD_ROWS);

    for (;;)
    {

        // Block until a UART trigger is received...
        BaseType_t taken = xSemaphoreTake(buttonEventSemaphore_, portMAX_DELAY);
        if (pdFAIL == taken)
        {
            continue;
        }

        xSemaphoreGive(dispatchEventSemaphore_);
    }
}


void ledTask(void *pvParameters)
{

    for (;;)
    {

        // Block until an event is dispatched...
        BaseType_t taken = xSemaphoreTake(dispatchEventSemaphore_, portMAX_DELAY);
        if (pdFAIL == taken)
        {
            continue;
        }


        // Then set the columns high so the interrupt can trigger again
        GPIOPinWrite(KEYPAD_PORT_BASE, KEYPAD_COLS, KEYPAD_COLS);

    }
}

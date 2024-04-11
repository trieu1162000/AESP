/*
 * systemTask.c
 *
 *  Created on: Jan 28, 2024
 *      Author: trieu
 */

#include <task.h>
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include "led.h"
#include "switch.h"
#include "system_task.h"

volatile uint32_t period_led_time = 0U;

//Interrupt service routine
void SWIntHandler(void){
	GPIOIntClear(SW_GPIO_BASE, GPIO_INT_PIN_0|GPIO_INT_PIN_4);
	int state = GPIOIntStatus(SW_GPIO_BASE, true);
    if( ( state & GPIO_INT_PIN_0 ) == GPIO_INT_PIN_0 )
    {
        // Give the SW1 semaphore indicating that it is being pressed
        // This will attempt a wake the higher priority SwitchTask and continue
        // execution there.
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        
        // Give the semaphore and unblock the SW1Task.
        xSemaphoreGiveFromISR(SW1PressedSemaphore_, &xHigherPriorityTaskWoken);
        
        // If the SW1Task was successfully woken, then yield execution to it
        // and go there now (instead of changing context to another task).
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    if( ( state & GPIO_INT_PIN_4 ) == GPIO_INT_PIN_4 )
    {
        // Give the SW2 semaphore indicating that it is being pressed
        // This will attempt a wake the higher priority switch task and continue
        // execution there.
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        
        // Give the semaphore and unblock the SW2Task.
        xSemaphoreGiveFromISR(SW2PressedSemaphore_, &xHigherPriorityTaskWoken);
        
        // If the SW2Task was successfully woken, then yield execution to it
        //	and go there now (instead of changing context to another task).
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void SW1Task(void *pvParameters)
{
    for (;;)
    {
        // Block until SW1 is trigger
        BaseType_t taken = xSemaphoreTake(SW1PressedSemaphore_, portMAX_DELAY);
        if (pdFAIL == taken)
        {
            continue;
        }

        // Extend the time period by 5 seconds
        period_led_time = period_led_time + (5000U / portTICK_PERIOD_MS);
    }
}

void SW2Task(void *pvParameters)
{
    for (;;)
    {
        // Block until SW2 is trigger
        BaseType_t taken = xSemaphoreTake(SW2PressedSemaphore_, portMAX_DELAY);
        if (pdFAIL == taken)
        {
            continue;
        }

        // Expire the time period constantly
        period_led_time = 0U;
    }
}


void ledTask(void *pvParameters)
{

    for (;;)
    {
        // Off all
        ledControl(LEDRED, OFF);
        ledControl(LEDGREEN, OFF);
        ledControl(LEDBLUE, OFF);

        // Turn on Green LED
        ledControl(LEDGREEN, ON)

        // Delay 15s
        period_led_time = LED_GREEN_TIME;
        vTaskDelay(period_led_time);

        // Turn off Green LED
        ledControl(LEDGREEN, OFF);

        // Turn on Yellow (Red + Blue) LED
        ledControl(LEDRED, ON);
        ledControl(LEDBLUE, ON);

        // Delay 3s
        period_led_time = LED_YELLOW_TIME;
        vTaskDelay(period_led_time);

        // Turn on Red LED (off Blue LED)
        ledControl(LEDBLUE, OFF);

        // Delay 15s
        period_led_time = LED_RED_TIME;
        vTaskDelay(period_led_time);
    }
}

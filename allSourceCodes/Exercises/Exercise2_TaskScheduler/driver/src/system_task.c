/*
 * systemTask.c
 *
 *  Created on: Jan 28, 2024
 *      Author: trieu
 */

#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include "led.h"
#include "switch.h"
#include "system_task.h"
#include "uartstdio.h"
#include "debug.h"

static const swTime_t sendSW1TimeVal = SW1_TIME;
static const swTime_t sendSW2TimeVal = SW2_TIME;
static TickType_t current_tick = 0U;
static TickType_t total_tick = 0U;
static TickType_t rest_tick = 0U;

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

        // Debouncing the switch about 20ms
        vTaskDelay( ( (TickType_t) 150U ) / portTICK_PERIOD_MS);

        // Enable the ext interrupt again
        GPIOIntClear(SW_GPIO_BASE, SW1_PIN | SW2_PIN);
        GPIOIntEnable(SW_GPIO_BASE, SW1_PIN | SW2_PIN);

        if( pdPASS != xQueueSend( timeEventQueue_,
                    ( void * ) &sendSW1TimeVal,
                    ( TickType_t ) 10 ) )
        {
            /* Failed to post the message, even after 10 ticks. */
        }

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

        // Debouncing the switch about 180ms
        vTaskDelay( ( (TickType_t) 180U ) / portTICK_PERIOD_MS);

        // Enable the ext interrupt again
        GPIOIntClear(SW_GPIO_BASE, SW1_PIN | SW2_PIN);
        GPIOIntEnable(SW_GPIO_BASE, SW1_PIN | SW2_PIN);

        if( pdPASS != xQueueSend( timeEventQueue_,
                    ( void * ) &sendSW2TimeVal,
                    ( TickType_t ) 10 ) )
        {
            /* Failed to post the message, even after 10 ticks. */
        }

    }
}


void ledTask(void *pvParameters)
{
    swTime_t recvTimeVal = (swTime_t) 0U;

    for (;;)
    {
        // Off all
        ledControl(LEDRED, OFF);
        ledControl(LEDGREEN, OFF);

        // Turn on Green LED
        ledControl(LEDGREEN, ON);

        // Get the current tick before delaying
        current_tick = xTaskGetTickCount();
        DBG("Current tick: %d\n", current_tick);

        // Reset the queue before recieve
        recvTimeVal = (swTime_t) 0U;

        // Delay 15s
        // Take message queue here
        xQueueReceive( timeEventQueue_,
                       &( recvTimeVal ),
                       ( TickType_t ) LED_GREEN_TIME );
        total_tick = LED_GREEN_TIME;

        if(recvTimeVal == SW1_TIME)
        {
            // do nothing
        }
        else if(recvTimeVal == SW2_TIME)
        {
            do
            {
                // Get the rest tick which is not being delayed
                rest_tick = current_tick + total_tick - xTaskGetTickCount();
                DBG("rest tick: %d\n", rest_tick);

                // Reset the queue before recieve again
                recvTimeVal = (swTime_t) 0U;

                // Delay the rest tick plus 5 seconds
                rest_tick = rest_tick + (TickType_t) ( 5000U / portTICK_PERIOD_MS );
                xQueueReceive( timeEventQueue_,
                            &( recvTimeVal ),
                            ( TickType_t ) rest_tick );
                total_tick += (TickType_t) ( 5000U / portTICK_PERIOD_MS );
            } while (recvTimeVal == SW2_TIME);


        }

        // Turn on Yellow (Red + Green) LED
        ledControl(LEDRED, ON);

        // Get the current tick before delaying
        current_tick = xTaskGetTickCount();
        DBG("Current tick: %d\n", current_tick);

        // Reset the queue before recieve
        recvTimeVal = (swTime_t) 0U;

        // Delay 3s
        xQueueReceive( timeEventQueue_,
                       &( recvTimeVal ),
                       ( TickType_t ) LED_YELLOW_TIME );
        total_tick = LED_YELLOW_TIME;

        if(recvTimeVal == SW1_TIME)
        {
            // do nothing
        }
        else if(recvTimeVal == SW2_TIME)
        {
            do
            {
                // Get the rest tick which is not being delayed
                rest_tick = current_tick + total_tick - xTaskGetTickCount();
                DBG("rest tick: %d\n", rest_tick);

                // Reset the queue before recieve again
                recvTimeVal = (swTime_t) 0U;

                // Delay the rest tick plus 5 seconds
                rest_tick = rest_tick + (TickType_t) ( 5000U / portTICK_PERIOD_MS );
                xQueueReceive( timeEventQueue_,
                            &( recvTimeVal ),
                            ( TickType_t ) rest_tick );
                total_tick += (TickType_t) ( 5000U / portTICK_PERIOD_MS );
            } while (recvTimeVal == SW2_TIME);
        }

        // Turn on Red LED (off Green LED)
        ledControl(LEDGREEN, OFF);

        // Get the current tick before delay
        current_tick = xTaskGetTickCount();
        DBG("Current tick: %d\n", current_tick);

        // Reset the queue before recieve
        recvTimeVal = (swTime_t) 0U;

        // Delay 15s
        xQueueReceive( timeEventQueue_,
                       &( recvTimeVal ),
                       ( TickType_t ) LED_RED_TIME );
        total_tick = LED_RED_TIME;

        if(recvTimeVal == SW1_TIME)
        {
            // do nothing
        }
        else if(recvTimeVal == SW2_TIME)
        {
            do
            {
                // Get the rest tick which is not being delayed
                rest_tick = current_tick + total_tick - xTaskGetTickCount();
                DBG("rest tick: %d\n", rest_tick);

                // Reset the queue before recieve again
                recvTimeVal = (swTime_t) 0U;

                // Delay the rest tick plus 5 seconds
                rest_tick = rest_tick + (TickType_t) ( 5000U / portTICK_PERIOD_MS );
                xQueueReceive( timeEventQueue_,
                            &( recvTimeVal ),
                            ( TickType_t ) rest_tick );
                total_tick += (TickType_t) ( 5000U / portTICK_PERIOD_MS );
            } while (recvTimeVal == SW2_TIME);
        }
    }
}

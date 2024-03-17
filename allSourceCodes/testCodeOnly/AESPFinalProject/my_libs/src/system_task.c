/*
 * systemTask.c
 *
 *  Created on: Jan 28, 2024
 *      Author: trieu
 */

#include "../inc/system_task.h"
#include <task.h>

alphaValue_t alpha_character_val = ALPHA_VALUE_FALSE;
bool is_alpha_character = false;
char pressed_key = NULL;
displayTaskValue_t display_task_val = DISPLAY_NONE;
bool check_result = true;

static void parseKey(char key)
{
    switch(key){
        case '*':
            is_alpha_character = !is_alpha_character;
            sFSM_.event_ = E_NONE;
            display_task_val = DISPLAY_APPEND;
            break;
        case '#':
            sFSM_.event_ = E_CACULATE;
            break;
        case '8':
            if(is_alpha_character)
            {
                sFSM_.event_ = E_RESET;

                // In addition, clear the is_alpha_character manually
                is_alpha_character = false;
                alpha_character_val = ALPHA_VALUE_FALSE;
            }
            else
                sFSM_.event_ = E_TYPED;
            break;
        default:
            sFSM_.event_ = E_TYPED;
            break;

    }
}

void buttonTask(void *pvParameters)
{
    // Clear the interrupt first since we have some unexpected error.
    // And this line is intended to fix this
    GPIOIntClear(KEYPAD_PORT_BASE, KEYPAD_ROWS);

    // Enable interrupts on these pins here
    GPIOIntEnable(KEYPAD_PORT_BASE, KEYPAD_ROWS);

    for (;;)
    {

        // Block until a UART trigger is received...
        BaseType_t taken = xSemaphoreTake(buttonEventSemaphore_, portMAX_DELAY);
        if (taken == pdFAIL)
        {
            continue;
        }

        pressed_key = getKeyOnKeypad(); // poll the keypad

        // Debounce for the keypad
        /* Block for 250ms. */
        const TickType_t xDelay = 250 / portTICK_PERIOD_MS;
        vTaskDelay( xDelay );

        if (pressed_key == NULL)
        {
            // if a symbol was pressed
            DBG("Error or invalid char\n");

            // Then set the columns high so the interrupt can trigger again
            GPIOPinWrite(KEYPAD_PORT_BASE, KEYPAD_COLS, KEYPAD_COLS);

            // Clear then enable the interrupt again
            GPIOIntClear(KEYPAD_PORT_BASE, KEYPAD_ROWS);
            GPIOIntEnable(KEYPAD_PORT_BASE, KEYPAD_ROWS);

            continue;
        }
        else
        {
            DBG("Char: %c\n", pressed_key);
        }

        // Parse the key to see which event is incoming
        parseKey(pressed_key);
    
        // // Clear then enable the interrupt again
        // GPIOIntClear(KEYPAD_PORT_BASE, KEYPAD_ROWS);

        // GPIOIntEnable(KEYPAD_PORT_BASE, KEYPAD_ROWS);

        xSemaphoreGive(dispatchEventSemaphore_);

    }
}

void mainTask(void *pvParameters)
{

    for (;;)
    {

        // Block until an event is dispatched...
        BaseType_t taken = xSemaphoreTake(dispatchEventSemaphore_, portMAX_DELAY);
        if (taken == pdFAIL)
        {
            continue;
        }

        // DBG("Bare task\n");

        switch (sFSM_.state_)
        {

        case S_STOPPED:
            switch (sFSM_.event_)
            {
            case E_TYPED:
                sFSM_.state_ = S_CONVERTING;
                DBG("State = S_CONVERTING\n");
                appendAction();
                display_task_val = DISPLAY_APPEND;
                break;
            case E_NONE:
                break;
            default:
                display_task_val = DISPLAY_NONE;
                break;
            }
            break;

        case S_CONVERTING:
            switch (sFSM_.event_)
            {
            case E_TYPED:
                sFSM_.state_ = S_CONVERTING;
                DBG("State = S_CONVERTING\n");
                appendAction();
                display_task_val = DISPLAY_APPEND;
                break;
            case E_RESET:
                sFSM_.state_ = S_STOPPED;
                DBG("State = S_STOPPED\n");
                clearAction();
                display_task_val = DISPLAY_CLEAR;
                break;
            case E_CACULATE:
                sFSM_.state_ = S_CACULATING;
                DBG("State = S_CACULATING\n");
                check_result = giveResultAction();
                display_task_val = DISPLAY_RESULT;
                break;
            case E_NONE:
                break;
            default:
                display_task_val = DISPLAY_NONE;
                break;
            }
            break;

        case S_CACULATING:
            switch (sFSM_.event_)
            {
            case E_TYPED:
                sFSM_.state_ = S_CONVERTING;
                DBG("State = S_CONVERTING\n");
                clearAction();
                clearDisplay(lcd);
                appendAction();
                display_task_val = DISPLAY_APPEND;
                break;
            case E_CACULATE:
                sFSM_.state_ = S_CACULATING;
                DBG("State = S_CACULATING\n");
                check_result = giveResultAction();
                display_task_val = DISPLAY_RESULT;
                break;
            case E_RESET:
                sFSM_.state_ = S_STOPPED;
                DBG("State = S_STOPPED\n");
                clearAction();
                display_task_val = DISPLAY_CLEAR;
                break;
            case E_NONE:
                break;
            default:
                display_task_val = DISPLAY_NONE;
                break;
            }
            break;
        }

        xSemaphoreGive(displayEventSemaphore_);

    }
}

void displayTask(void *pvParameters)
{

   for (;;)
   {

        // Block until an event is dispatched...
        BaseType_t taken = xSemaphoreTake(displayEventSemaphore_, portMAX_DELAY);
        if (taken == pdFAIL)
        {
            continue;
        }

        switch (display_task_val)
        {
        case DISPLAY_APPEND:
            appendDisplay(lcd);
            break;
        case DISPLAY_CLEAR:
            clearDisplay(lcd);
            DBG("Cleared\n");
            break;
        case DISPLAY_RESULT:
            resultDisplay(lcd);
            break;
        default:
            break;
        }

        // Then set the columns high so the interrupt can trigger again
        GPIOPinWrite(KEYPAD_PORT_BASE, KEYPAD_COLS, KEYPAD_COLS);

        // Clear then enable the interrupt again
        GPIOIntClear(KEYPAD_PORT_BASE, KEYPAD_ROWS);
        GPIOIntEnable(KEYPAD_PORT_BASE, KEYPAD_ROWS);
    }
}


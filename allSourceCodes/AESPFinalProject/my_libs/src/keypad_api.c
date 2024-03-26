/*
 * keypad_api.c
 *
 *  Created on: Jan 29, 2024
 *      Author: trieu
 */

#include "../inc/keypad_api.h"

static const char chars_on_keypad[KEYPAD_ROW_COUNT][KEYPAD_COL_COUNT] = {
    {0x31, 0x32, 0x33, 0x41}, // This row stands for: 1, 2, 3, A with hex value
    {0x34, 0x35, 0x36, 0x42}, // This row stands for: 4, 5, 6, B with hex value
    {0x37, 0x38, 0x39, 0x43}, // This row stands for: 7, 8, 9, C with hex value
    {0x2A, 0x30, 0x23, 0x44}, // This row stands for: *, 0, #, D with hex value
};

static uint8_t getRow(unsigned char check_column)
{
    uint8_t row_pin = 0U;

    // Unreach return variable
    uint8_t ret = 5U;

    GPIOPinWrite(KEYPAD_PORT_BASE, KEYPAD_COLS, 0U);
    GPIOPinWrite(KEYPAD_PORT_BASE, KEYPAD_COLS, check_column);
    row_pin = (uint8_t) GPIOPinRead(KEYPAD_PORT_BASE, KEYPAD_ROWS);

    // First row, but in 0th position in array
    if (row_pin & KEYPAD_ROW_1ST)
    {
        ret = 0U;
    }
    // Second row, but in 1st position in array
    else if (row_pin & KEYPAD_ROW_2ND)
    {
        ret = 1U;
    }
    // Third row, but in 2nd position in array
    else if (row_pin & KEYPAD_ROW_3RD)
    {
        ret = 2U;
    }
    // Fourth row, but in 3rd position in array
    else if (row_pin & KEYPAD_ROW_4TH)
    {
        ret = 3U;
    }
    // Handling the error if any
    return ret;
}

void initKeypad(uint32_t ui32Port)
{
    GPIOPinTypeGPIOOutput(
        ui32Port,
        KEYPAD_COLS);

    GPIOPinTypeGPIOInput(
        ui32Port,
        KEYPAD_ROWS);

    // turn on all columns
    // do this so that when any key is pressed, a rising edge will happen on Port B and
    // trigger an interrupt. The ISR polls column-by-column to see key in particular it is.
    GPIOPinWrite(
        ui32Port,
        KEYPAD_COLS,
        KEYPAD_COLS);
    GPIOPadConfigSet(ui32Port, KEYPAD_ROWS, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
    GPIOIntTypeSet(ui32Port, KEYPAD_ROWS, GPIO_RISING_EDGE); // set the interrupt type on these pins
    GPIOIntRegister(ui32Port, keypadIntHandler);

}

char getKeyOnKeypad(void)
{
    uint8_t row = 0U;
    uint8_t col = 0U;

    row = getRow(KEYPAD_COL_1ST);
    if (5U != row)
    {
        col = 0U;
        GPIOIntClear(KEYPAD_PORT_BASE, KEYPAD_ROWS);
        return chars_on_keypad[row][col];
    }

    row = getRow(KEYPAD_COL_2ND);
    if (5U != row)
    {
        col = 1U;
        GPIOIntClear(KEYPAD_PORT_BASE, KEYPAD_ROWS);
        return chars_on_keypad[row][col];
    }

    row = getRow(KEYPAD_COL_3RD);
    if (5U != row)
    {
        col = 2U;
        GPIOIntClear(KEYPAD_PORT_BASE, KEYPAD_ROWS);
        return chars_on_keypad[row][col];
    }

    row = getRow(KEYPAD_COL_4TH);
    if (5U != row)
    {
        col = 3U;
        GPIOIntClear(KEYPAD_PORT_BASE, KEYPAD_ROWS);
        return chars_on_keypad[row][col];
    }

    // Handling the error if any
    GPIOIntClear(KEYPAD_PORT_BASE, KEYPAD_ROWS);
    return NULL;
}

void keypadIntHandler(void)
{
    // Disable the interrupt first
    GPIOIntDisable(KEYPAD_PORT_BASE, KEYPAD_ROWS);

    // This will attempt to wake the ButtonTask and continue execution there.
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Give the semaphore and unblock the MessageTask.
    xSemaphoreGiveFromISR(buttonEventSemaphore_, &xHigherPriorityTaskWoken);

    // If the ButtonTask was successfully woken, then yield execution to it
    //    and go there now (instead of changing context to another task).
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

}

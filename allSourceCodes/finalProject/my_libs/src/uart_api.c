/*
 * uart_api.c
 *
 *  Created on: Oct 27, 2023
 *      Author: trieu
 */


//*****************************************************************************
//
// These UART API are supposed to be used for 2 purposes: 
// Debug logs in terminal
// Transmit/recieve to/from Desktop App by CC2530 Module
//
//*****************************************************************************

#include "../inc/uart_api.h"

uint8_t rawReceivedFrame[MAX_FRAME_LENGTH] = {0};
uint8_t mainFrame[MAX_FRAME_LENGTH] = {0};
uint8_t receivedFrameIndex = 0;

void UARTStringPut(uint32_t ui32Base,const char *str){

    size_t count;
    size_t i;
    char ch;

    count = strlen(str);
    for(i = 0; i < count; i++){
        ch = str[i];
        UARTCharPut(ui32Base, (char) ch );
    }
}

// Using UART1
void UARTIntHandler(void)
{
    uint32_t ui32Status;
    int16_t i = 0;

    //
    // Get the interrrupt status.
    //
    ui32Status = UARTIntStatus(UART1_BASE, true);

    //
    // Clear the asserted interrupts.
    //
    UARTIntClear(UART1_BASE, ui32Status);

    //
    // Loop while there are characters in the receive FIFO.
    //
    while (UARTCharsAvail(UART1_BASE))
    {
        uint8_t receivedByte = UARTCharGetNonBlocking(UART1_BASE);

        rawReceivedFrame[i] = receivedByte;
        i++;
    }
    int j;
    for (j = 0; j < i && receivedFrameIndex < MAX_FRAME_LENGTH; j++) {
        mainFrame[receivedFrameIndex] = rawReceivedFrame[j];
        receivedFrameIndex++;
    }

    if(receivedFrameIndex > 4)
    {
        // This will attempt to wake the MessageTask and continue execution there.
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        
        // Give the semaphore and unblock the MessageTask.
        xSemaphoreGiveFromISR(buttonEventSemaphore_, &xHigherPriorityTaskWoken);
        
        // If the MessageTask was successfully woken, then yield execution to it
        //	and go there now (instead of changing context to another task).
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

}

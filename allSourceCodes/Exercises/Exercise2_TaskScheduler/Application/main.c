#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <stdbool.h>
#include <time.h>
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "inc/hw_memmap.h"
#include "system_task.h"
#include "switch.h"
#include "led.h"
#include "uartstdio.h"
#include "debug.h"

// Binary Semaphores
xSemaphoreHandle SW1PressedSemaphore_;
xSemaphoreHandle SW2PressedSemaphore_;

// Message Queue
xQueueHandle timeEventQueue_;

//*****************************************************************************
//
// This hook is called by FreeRTOS when an stack overflow error is detected.
//
//*****************************************************************************

void vApplicationIdleHook()
{
    // do nothing
}

void
vApplicationStackOverflowHook(xTaskHandle *pxTask, char *pcTaskName)
{
    //
    // This function can not return, so loop forever.  Interrupts are disabled
    // on entry to this function, so no processor interrupts will interrupt
    // this loop.
    //
    while(1)
    {
        // Spin...
    }
}

#if 1
// Catch-all error handler.
static void errHandler(void)
{
        // Spin...
        while (1);
}
#endif

// Init the UART just used for debugging, comment out the macro in debug.h if not using
#ifdef DEBUG
void initConsole(void)
{
    //
    // Enable GPIO port A which is used for UART0 pins.
    // TODO: change this to whichever GPIO port you are using.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Configure the pin muxing for UART0 functions on port A0 and A1.
    // This step is not necessary if your part does not support pin muxing.
    // TODO: change this to select the port/pin you are using.
    //
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    //
    // Enable UART0 so that we can configure the clock.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Select the alternate (UART) function for these pins.
    // TODO: change this to select the port/pin you are using.
    //
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}
#endif

int initTasks()
{

    int8_t success = -1;

    do {

        // Create the semaphores
        SW1PressedSemaphore_ = xSemaphoreCreateBinary();
        if (!SW1PressedSemaphore_) {
            break;
        }

        SW2PressedSemaphore_ = xSemaphoreCreateBinary();
        if (!SW2PressedSemaphore_) {
            break;
        }

        // Create the queue
        timeEventQueue_ = xQueueCreate( 1, sizeof( swTime_t ) );

        if(!timeEventQueue_)
        {
            /* Queue was not created and must not be used. */
            break;
        }


        // Create the tasks
        // The SW tasks will have a highest priority
        xTaskCreate(SW1Task, "SW1 Handler", STACK_SIZE, NULL, 2, NULL);
        xTaskCreate(SW2Task, "SW2 Handler", STACK_SIZE, NULL, 2, NULL);

        // The LED task will have a lower priority than the SW tasks.
        xTaskCreate(ledTask, "LEDs Hanlder", STACK_SIZE, NULL, 1, NULL);

        success = 0;

    }    while (pdFALSE);


    return success;

}



int main(void) {

    // Set the system clock
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // Init switches
    switchInit();

    // Init LEDs
    ledInit();

#ifdef  DEBUG
    initConsole();
#endif

    // Init tasks for FreeRTOS
     if (initTasks()) {
        errHandler();
     }

     // Enable processor interrupts
    IntMasterEnable();

     // Startup of the FreeRTOS scheduler.  The program should block here.
     vTaskStartScheduler();

    // The following line should never be reached.  Failure to allocate enough
    //    memory from the heap would be one reason.
    for (;;)
    {

    }

}




#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <stdbool.h>
#include <time.h>
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/hibernate.h"
#include "inc/hw_memmap.h"
#include "../my_libs/inc/config_peripherals_api.h"
#include "../my_libs/inc/system_task.h"

// Binary Semaphores
xSemaphoreHandle SW1PressedSemaphore_;
xSemaphoreHandle SW2PressedSemaphore_;

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

    // Init all related peripherals
    initPeriphs();

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




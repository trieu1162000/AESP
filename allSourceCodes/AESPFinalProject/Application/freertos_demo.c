#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "../my_libs/inc/config_peripherals_api.h"
#include "../my_libs/inc/lcd_i2c_api.h"
#include "../my_libs/inc/states.h"
#include "../my_libs/inc/actions_api.h"
#include "../my_libs/inc/system_FSM_api.h"
#include "../my_libs/inc/system_task.h"
#include "../my_libs/inc/keypad_api.h"
#include "../my_libs/inc/caculator_api.h"

// This is used sFSM
SemaphoreHandle_t acceptEventSemaphore_;
SemaphoreHandle_t dispatchEventSemaphore_;
SemaphoreHandle_t displayEventSemaphore_;
SemaphoreHandle_t buttonEventSemaphore_;

struct lcd_i2c *lcd = NULL;
caculator_t *sCaculator_ = NULL;

//*****************************************************************************
//
// This hook is called by FreeRTOS when an stack overflow error is detected.
//
//*****************************************************************************
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

    int success = -1;

    do {

        acceptEventSemaphore_ = xSemaphoreCreateBinary();
        if (!acceptEventSemaphore_) {
            break;
        }

        dispatchEventSemaphore_ = xSemaphoreCreateBinary();
        if (!dispatchEventSemaphore_) {
            break;
        }

        buttonEventSemaphore_ = xSemaphoreCreateBinary();
        if (!buttonEventSemaphore_) {
            break;
        }

        displayEventSemaphore_ = xSemaphoreCreateBinary();
        if (!displayEventSemaphore_) {
            break;
        }

        // The UART task will have a highest priority
        xTaskCreate(buttonTask, "Button Handler", STACK_SIZE * 3, NULL, 3, NULL);

        // The Bare task will have a higher priority than the Polling task.
        xTaskCreate(mainTask, "Main Hanlder", STACK_SIZE * 2, NULL, 2, NULL);

        // Create the track tasks at the lowest priority.
        xTaskCreate(displayTask, "Display Handler", STACK_SIZE, NULL, 1, NULL);

        success = 0;

    }    while (pdFALSE);


    return success;

}



int main(void) {

    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // Init all related peripherals
    initPeriphs();

    // Init I2C protocol
    initI2C();

#ifdef  DEBUG
    initConsole();
#endif

    // Init the LCD and its parameters
    lcd = malloc(sizeof(*lcd));
    initParamsLCD(lcd, lcd_i2c_geometries[1]);
    initLCD(lcd);
    
    // Init the keypad
    initKeypad(KEYPAD_PORT_BASE);

    // Init all parameter for caculator
    sCaculator_ = malloc(sizeof(*sCaculator_));
    initParamsCaculator(sCaculator_);

    // Init tasks for FreeRTOS
     if (initTasks()) {
        errHandler();
     }

     // Init parameters for FSM
     sFSM_.event_ = E_RESET;
     sFSM_.state_ = S_STOPPED;

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




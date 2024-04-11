#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "../my_libs/inc/config_peripherals_api.h"
#include "../my_libs/inc/rc522_api.h"
#include "../my_libs/inc/uart_api.h"
#include "../my_libs/inc/lcd_i2c_api.h"
#include "../my_libs/inc/timer_handler_api.h"
#include "../my_libs/inc/eeprom_api.h"
#include "../my_libs/inc/states.h"
#include "../my_libs/inc/actions_api.h"

// Minimum stack size for FreeRTOS tasks.
#define STACK_SIZE 100

struct lcd_i2c *lcd = NULL;

// This is used by bFSM
SemaphoreHandle_t bTimerEventSemaphore_;
SemaphoreHandle_t acceptEventSemaphore_;
SemaphoreHandle_t dispatchEventSemaphore_;

// This is used by gFSM
SemaphoreHandle_t gTimerEventSemaphore_;
SemaphoreHandle_t gAcceptEventSemaphore_;
SemaphoreHandle_t displayEventSemaphore_;
SemaphoreHandle_t buttonEventSemaphore_;
SemaphoreHandle_t pollingEventSemaphore_;

// This is used by both
SemaphoreHandle_t switchGUIEventSemaphore_;
SemaphoreHandle_t switchBareEventSemaphore_;

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

void buttonTask(void *pvParameters)
{

    for (;;) {

        // Block until a UART trigger is received...
        BaseType_t taken = xSemaphoreTake(buttonEventSemaphore_, portMAX_DELAY);
        if (taken == pdFAIL) {
            continue;
        }


    }
}

void mainTask(void *pvParameters)
{

    for (;;) {

        xSemaphoreGive(acceptEventSemaphore_);

        // Block until an event is dispatched...
        BaseType_t taken = xSemaphoreTake(dispatchEventSemaphore_, portMAX_DELAY);
        if (taken == pdFAIL) {
            continue;
        }

        // DBG("Bare task\n");

        switch (sFSM_.state_)
        {

            case S_STOPPED:
                switch (sFSM_.event_) {
                    default:
                        break;
                }
                break;

            case S_CONVERTING:
                switch (sFSM_.event_) {
                    default:
                        break;
                }
                break;

            case S_CACULATING:
                switch (sFSM_.event_) {
                    default:
                        break;
                }
                break;

        }
    }
}


void displayTask(void *pvParameters)
{

    for (;;) {



    }
}

// Catch-all error handler.
static void errHandler(void)
{
        // Spin...
        while (1);
}

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

    initPeriphs();
    initBuzzer();
    initTimer();
#ifdef  DEBUG
    initConsole();
#endif

    sFSM_.event_ = E_FINISHED;
    sFSM_.state_ = S_STOPPED;

    lcd = malloc(sizeof(*lcd));
    initParamsLCD(lcd, lcd_i2c_geometries[0]);
    lcdInit(lcd);

    lcdSetCursorBlink(lcd, false);
    lcdSetCursorDisplay(lcd, false);
    normalDisplay();
    
    //======================================================================================

    if (initTasks()) {
       errHandler();
    }

    // Startup of the FreeRTOS scheduler.  The program should block here.
    vTaskStartScheduler();

    // The following line should never be reached.  Failure to allocate enough
    //    memory from the heap would be one reason.
    for (;;);

}




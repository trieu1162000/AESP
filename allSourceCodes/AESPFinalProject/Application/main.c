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

#define TIME_OUT    7000    // 7s

// Binary Semaphores
xSemaphoreHandle acceptEventSemaphore_;
xSemaphoreHandle dispatchEventSemaphore_;
xSemaphoreHandle buttonEventSemaphore_;

// Single-element Queue
xQueueHandle displayEventQueue_;

// Definition of global variables, must be initialized beforme entering the main program
uint32_t task_idle_count = 0U;
struct lcd_i2c *lcd = NULL;
caculator_t *sCaculator_ = NULL;

//*****************************************************************************
//
// This hook is called by FreeRTOS when an stack overflow error is detected.
//
//*****************************************************************************

void vApplicationIdleHook()
{

    if(0U == task_idle_count)
        task_idle_count = xTaskGetTickCount();

    // Get the subtraction to caculate the time
    int32_t sub = (int32_t ) (xTaskGetTickCount() - task_idle_count);

    // Check whether the time over the thresh hold, the system will be switched to hibernate mode
    if(sub > TIME_OUT){
        DBG("Time: %d\n", xTaskGetTickCount() - task_idle_count);
        task_idle_count = 0U;

        // Clear the display and set back light off
        lcdClearDisplay(lcd);
        lcdSetBackLight(lcd, false);

        // Set the cursor off, by the way
        lcdSetCursorDisplay(lcd, false);

        // Then, go to the hibernate mode
        HibernateRequest();
    }
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

        displayEventQueue_ = xQueueCreate( 1, sizeof( displayTaskValue_t ) );

        if(!displayEventQueue_)
        {
            /* Queue was not created and must not be used. */
            break;
        }

        // The button task will have a highest priority
        xTaskCreate(buttonTask, "Button Handler", STACK_SIZE * 3, NULL, 3, NULL);

        // The main task will have a lower priority than the button task.
        xTaskCreate(mainTask, "Main Hanlder", STACK_SIZE * 2, NULL, 2, NULL);

        // Create the display tasks at the lowest priority.
        xTaskCreate(displayTask, "Display Handler", STACK_SIZE, NULL, 1, NULL);

        success = 0;

    }    while (pdFALSE);


    return success;

}



int main(void) {

    // Set the system clock
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // Init hibernate mode
    initHibernateMode();

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

    // Get the current tick before starting the scheduler
     task_idle_count = xTaskGetTickCount();

     // Startup of the FreeRTOS scheduler.  The program should block here.
     vTaskStartScheduler();

    // The following line should never be reached.  Failure to allocate enough
    //    memory from the heap would be one reason.
    for (;;)
    {

    }

}




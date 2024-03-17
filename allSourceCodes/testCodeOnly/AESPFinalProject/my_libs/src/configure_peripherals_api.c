/*
 * configure_peripherals_api.c
 *
 *  Created on: Oct 27, 2023
 *      Author: trieu
 */

/*
 *  This is the connection between TIVA C and all modules used in this project
 */
//                    TIVA C             Connector           DESCRIPTION
//  //****************I2C LCD *********************************************
// GND
// 5V
// SDA                PA7                J1                 I2C1SDA             
// SCL                PA6                J1                 I2C1SCL             
//  //****************Keypad***********************************************
// IN                 PE5                J1                 GPIO Input      
// IN                 PE5                J1                 GPIO Input      
// IN                 PE5                J1                 GPIO Input      
// IN                 PE5                J1                 GPIO Input                     
// IN                 PE5                J1                 GPIO Output      
// IN                 PE5                J1                 GPIO Output      
// IN                 PE5                J1                 GPIO Output      
// IN                 PE5                J1                 GPIO Output      


#include "../inc/config_peripherals_api.h"

void initPeriphs(void){
    // Enable for LED Green, I2C LCD
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Enable for LED Red, UART CC2530
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

}

// Init the I2C used for LCD
void initI2C(void){
    //
    // Enable the I2C0 peripheral
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C1);
    GPIOPinConfigure(GPIO_PA6_I2C1SCL);
    GPIOPinConfigure(GPIO_PA7_I2C1SDA);
    GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, GPIO_PIN_6);
    GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_7);
    //
    // Initialize Master and Slave
    //
    I2CMasterInitExpClk(I2C1_BASE, SysCtlClockGet(), true);
    HWREG(I2C1_BASE + I2C_O_FIFOCTL) = 80008000;

}

// Init the UART just used for debugging, comment out the macro in debug.h if not using
#ifdef DEBUG
void initConsole(void)
{
    //
    // Enable GPIO port A which is used for UART0 pins.
    // TODO: change this to whichever GPIO port you are using.
    //
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

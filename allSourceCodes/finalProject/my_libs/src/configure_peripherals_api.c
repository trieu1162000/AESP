/*
 * configure_peripherals_api.c
 *
 *  Created on: Oct 27, 2023
 *      Author: trieu
 */

/*
 *  This is the connection between TIVA C and all modules used in this project
 */
//                    TIVA C            Connector           DESCRIPTION
//  //****************RFID RC522*******************************************
// CS (SDA)           PD1               J3                  SSI3Fss             
// SCK                PD0               J3                  SSI3CLK             
// MOSI               PD3               J3                  SSI3Tx              
// MISO               PD2               J3                  SSI3Rx              
// IRQ                PE1               J3                  GPIO Input - Irq
// GND                GND               J3                  Ground
// RST                3.3V              J3                  Reset pin (3.3V)
// VCC                3.3V              J3                  3.3V power
//  //****************I2C LCD *********************************************
// GND
// 5V
// SDA                PA7               J1                  I2C1SDA             
// SCL                PA6               J1                  I2C1SCL             
//  //****************CC2530 UART******************************************
// GND
// RX                 PB1               J1                  U1TX                   
// TX                 PB0               J1                  U1RX               
// 5V
//  //****************Buzzer***********************************************
// GND
// I/O                PE4                J1                 GPIO Output       
// 5V

//  //****************Relay - Lock******************************************
// IN                 PE5                J1                 GPIO Output      
// 5V
// GND
//  //****************LEDs**************************************************
// Green              PA5                J1                 GPIO Output     
// GND
// Red                PB4                J1                 GPIO Output      
// GND

#include "../inc/config_peripherals_api.h"

void initPeriphs(void){
    // Enable for LED Green, I2C LCD
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Enable for LED Red, UART CC2530
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // Enable for RFID RC522
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

    // Enable for Relay - Lock
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    // Enable for INT TIMER
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);

    // Enable processor interrupts
    IntMasterEnable();

}

void initLEDs(void)
{
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_5); // PA5
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_4); // PB4
}

void initLock(void)
{
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_5); // PE5
}

void initBuzzer(void)
{
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_4); // PE4
}

void initTimer(void)
{
    TimerConfigure(TIMER1_BASE, TIMER_CFG_ONE_SHOT);
    TimerLoadSet(TIMER1_BASE, TIMER_A, 3*SysCtlClockGet()-1);
    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    TimerConfigure(TIMER0_BASE, TIMER_CFG_ONE_SHOT);
    TimerLoadSet(TIMER0_BASE, TIMER_A, 3*SysCtlClockGet()-1);
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    TimerConfigure(TIMER2_BASE, TIMER_CFG_ONE_SHOT);
    TimerLoadSet(TIMER2_BASE, TIMER_A, 6*SysCtlClockGet()-1);
    IntEnable(INT_TIMER2A);
    TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
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

// Init SPI used for RFID
void initSPI(void){
    uint32_t junkAuxVar;

    // TBD. Need to accordingly change Port and Pins.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI3);

    GPIOPinConfigure(GPIO_PD0_SSI3CLK);
//    GPIOPinConfigure(GPIO_PD1_SSI3FSS);
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_1); // CS Pin
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_2); // IRQ Pin
    GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    GPIOPinConfigure(GPIO_PD2_SSI3RX);
    GPIOPinConfigure(GPIO_PD3_SSI3TX);
    GPIOPinTypeSSI(GPIO_PORTD_BASE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_0);
    SSIConfigSetExpClk(SSI3_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 4000000, 8);

    SSIEnable(SSI3_BASE);

    while(SSIDataGetNonBlocking(SSI3_BASE, &junkAuxVar)){}

}

// Init the UART used for CC2530
void initUART(void){

    // TBD. Need to update for using two UART Peripherals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);

    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 9600,
        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    
    //
    // Enable the UART interrupt.
    //
    IntEnable(INT_UART1);
    UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);
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

/*
 * rc522_api.c
 *
 *  Created on: Oct 27, 2023
 *      Author: trieu
 */


// MFRC522            TIVA C                    DESCRIPTION
// CS (SDA)           PD1                       SSI3Fss             Chip select for SPI
// SCK                PD0                       SSI3CLK             Serial Clock for SPI
// MOSI               PD3                       SSI3Tx              Master In Slave Out for SPI
// MISO               PD2                       SSI3Rx              Master Out Slave In for SPI
// IRQ                -                         Irq
// GND                GND                       Ground
// RST                3.3V                      Reset pin (3.3V)
// VCC                3.3V                      3.3V power

#include "../inc/rc522_api.h"

static void lowCSPin(void){
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, LOW_PIN);

}

static void highCSPin(void){
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, HIGH_PIN);

}

static uint8_t spiTransfer(uint8_t data){
    uint32_t rxtxData;
    rxtxData = data;

    SSIDataPut(SSI3_BASE, (uint8_t) rxtxData);

    while(SSIBusy(SSI3_BASE));

    SSIDataGet(SSI3_BASE, &rxtxData);

    return (uint8_t) rxtxData;
}
/*
 * Function: rc522WriteRaw
 * Description: write a byte data into one register of MFRC522
 * Input parameter: ucAddress--register address；ucValue--the value that need to write in
 * Return: NULL
 */
void rc522WriteRaw(uint8_t ucAddress, uint8_t ucValue) {
    unsigned char ucAddr;

    lowCSPin();
    ucAddr = ((ucAddress << 1) & 0x7E);

//    SSIDataPut(SSI3_BASE, ucAddr);
//    SSIDataPut(SSI3_BASE, ucValue);
    spiTransfer(ucAddr);
    spiTransfer(ucValue);
//    while(SSIBusy(SSI3_BASE)){
//    }

    highCSPin();
}

/*
 * Function: rc522ReadRaw
 * Description: read a byte data into one register of MFRC522
 * Input parameter: ucAddr--register address
 * Return: return the read value
 */
static uint8_t rc522ReadRaw(uint8_t ucAddress) {
    uint8_t ucValue, ucAddr;

    lowCSPin();
    ucAddr = (((ucAddress << 1) & 0x7E) | 0x80);
//    SSIDataPut(SSI3_BASE, ucAddr);
//    SSIDataGet(SSI3_BASE, (uint32_t *) &ucValue);

    spiTransfer(ucAddr);
    ucValue = spiTransfer(0x00);

    highCSPin();

    return ucValue;
}

int8_t rc522Check(uint8_t* id) {
    int8_t status;

    status = rc522Request(PICC_REQIDL, id); // Find cards, return card type
    if (status == MI_OK)
        status = rc522Anticoll(id); // Card detected. Anti-collision, return card serial number 4 bytes
    rc522Halt(); // Command card into hibernation 
    return status;
}

/*
 * Function: rc522SetBitMask
 * Description: set RC522 register bit
 * Input parameter: reg--register address;mask--value
 * Return: NULL
 */
static void rc522SetBitMask(uint8_t reg, uint8_t mask) {
    rc522WriteRaw(reg, rc522ReadRaw(reg) | mask);
}

/*
 * Function: rc522ClearBitMask
 * Description: clear RC522 register bit
 * Input parameter: reg--register address;mask--value
 * Return: NULL
 */
static void rc522ClearBitMask(uint8_t reg, uint8_t mask){
    rc522WriteRaw(reg, rc522ReadRaw(reg) & (~mask));
}

/*
 * Function: rc522Request
 * Description: Searching card, read card type
 * Input parameter: reqMode--search methods，
 *                  TagType--return card types
 *                  0x4400 = Mifare_UltraLight
 *                  0x0400 = Mifare_One(S50)
 *                  0x0200 = Mifare_One(S70)
 *                  0x0800 = Mifare_Pro(X)
 *                  0x4403 = Mifare_DESFire
 * return: MI_OK if successed
 */
int8_t rc522Request(uint8_t reqMode, uint8_t* tagType) {
    int8_t status;
    uint16_t backBits; // The received data bits

    rc522WriteRaw(RC522_REG_BIT_FRAMING, 0x07); // TxLastBists = BitFramingReg[2..0]
    tagType[0] = reqMode;
    status = rc522ToCard(PCD_TRANSCEIVE, tagType, 1, tagType, &backBits);
    if ((status != MI_OK) || (backBits != 0x10)) 
    {
        status = MI_ERR;
    }

    return status;

}

/*
 * Function: rc522ToCard
 * Description: communicate between RC522 and ISO14443
 * Input parameter: command--MF522 command bits
 *             sendData--send data to card via rc522
 *             sendLen--send data length
 *             backData--the return data from card
 *             backLen--the length of return data
 * Return: return MI_OK if successed
 */
static int8_t rc522ToCard(uint8_t command, uint8_t* sendData, uint8_t sendLen, uint8_t* backData, uint16_t* backLen) {
    int8_t status = MI_ERR;
    uint8_t irqEn = 0x00;
    uint8_t waitIRq = 0x00;
    uint8_t lastBits;
    uint8_t n;
    uint16_t i;

    switch (command) {
        case PCD_AUTHENT: {
            irqEn = 0x12;
            waitIRq = 0x10;
            break;
        }
        case PCD_TRANSCEIVE: {
            irqEn = 0x77;
            waitIRq = 0x30;
            break;
        }
        default:
            break;
    }

//    rc522WriteRaw(RC522_REG_COMM_IE_N, irqEn | 0x80);
    rc522ClearBitMask(RC522_REG_COMM_IRQ, 0x80);
    rc522SetBitMask(RC522_REG_FIFO_LEVEL, 0x80);
    rc522WriteRaw(RC522_REG_COMMAND, PCD_IDLE);

    // Writing data to the FIFO
    for (i = 0; i < sendLen; i++) 
        rc522WriteRaw(RC522_REG_FIFO_DATA, sendData[i]);

    // Execute the command
    rc522WriteRaw(RC522_REG_COMMAND, command);
    if (command == PCD_TRANSCEIVE) 
        rc522SetBitMask(RC522_REG_BIT_FRAMING, 0x80); // StartSend=1,transmission of data starts

    // Waiting to receive data to complete
    i = 10000; // i according to the clock frequency adjustment, the operator M1 card maximum waiting time 25ms
    do {
        // CommIrqReg[7..0]
        // Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
        n = rc522ReadRaw(RC522_REG_COMM_IRQ);
        i--;
    } while ((i!=0) && !(n&0x01) && !(n&waitIRq));

    rc522ClearBitMask(RC522_REG_BIT_FRAMING, 0x80); // StartSend=0

    if (i != 0)  {
        if (!(rc522ReadRaw(RC522_REG_ERROR) & 0x1B)) {
            status = MI_OK;
            if (n & irqEn & 0x01)
                status = MI_NOTAGERR;
            if (command == PCD_TRANSCEIVE) {
                n = rc522ReadRaw(RC522_REG_FIFO_LEVEL);
                lastBits = rc522ReadRaw(RC522_REG_CONTROL) & 0x07;
                if (lastBits) 
                    *backLen = (n-1)*8+lastBits;
                else
                    *backLen = n*8;

                if (n == 0) 
                    n = 1;

                if (n > RC522_MAX_LEN)
                    n = RC522_MAX_LEN;

                for (i = 0; i < n; i++) 
                    backData[i] = rc522ReadRaw(RC522_REG_FIFO_DATA); // Reading the received data in FIFO
            }
        } else {
            status = MI_ERR;
        }
    }
    return status;
}

/*
 * Function: rc522Anticoll
 * Description: Prevent conflict, read the card serial number 
 * Input parameter: serNum--return the 4 bytes card serial number, the 5th byte is recheck byte
 * Return: return MI_OK if successed
 */
int8_t rc522Anticoll(uint8_t* serNum) {
    int8_t status;
    uint8_t i;
    uint8_t serNumCheck = 0;
    uint16_t unLen;

    rc522WriteRaw(RC522_REG_BIT_FRAMING, 0x00); // TxLastBists = BitFramingReg[2..0]
    serNum[0] = PICC_ANTICOLL;
    serNum[1] = 0x20;
    status = rc522ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);
    if (status == MI_OK) {
        // Check card serial number
        for (i = 0; i < 4; i++) 
            serNumCheck ^= serNum[i];
        if (serNumCheck != serNum[i]) 
            status = MI_ERR;
    }

    return status;

}

/*
 * Function: rc522CalculateCRC
 * Description: Use MFRC522 to calculate CRC
 * Input parameter: pIndata--the CRC data need to be read，len--data length，pOutData-- the caculated result of CRC
 * Return: NULL
 */
static void rc522CalculateCRC(uint8_t*  pIndata, uint8_t len, uint8_t* pOutData) {
    uint8_t i, n;

    rc522ClearBitMask(RC522_REG_DIV_IRQ, 0x04); // CRCIrq = 0
    rc522SetBitMask(RC522_REG_FIFO_LEVEL, 0x80); // Clear the FIFO pointer
    // Write_rc522(CommandReg, PCD_IDLE);

    // Writing data to the FIFO    
    for (i = 0; i < len; i++) 
        rc522WriteRaw(RC522_REG_FIFO_DATA, *(pIndata+i));

    rc522WriteRaw(RC522_REG_COMMAND, PCD_CALCCRC);

    // Wait CRC calculation is complete
    i = 0xFF;
    do {
        n = rc522ReadRaw(RC522_REG_DIV_IRQ);
        i--;
    } while ((i!=0) && !(n&0x04)); // CRCIrq = 1

    // Read CRC calculation result
    pOutData[0] = rc522ReadRaw(RC522_REG_CRC_RESULT_L);
    pOutData[1] = rc522ReadRaw(RC522_REG_CRC_RESULT_M);
}

/*
 * Function: rc522SelectTag
 * Description: Select card, read card storage volume
 * Input parameter: serNum--Send card serial number
 * Return: return the card storage volume
 */
int8_t rc522SelectTag(uint8_t* serNum) {
    uint8_t i;
    int8_t status;
    uint8_t size;
    uint16_t recvBits;
    uint8_t buffer[9]; 

    buffer[0] = PICC_SELECTTAG;
    buffer[1] = 0x70;

    for (i = 0; i < 5; i++) 
        buffer[i+2] = *(serNum+i);
    rc522CalculateCRC(buffer, 7, &buffer[7]); //??

    status = rc522ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);
    if ((status == MI_OK) && (recvBits == 0x18)) 
        size = buffer[0]; 
    else 
        size = 0;
    return size;
}

/*
 * Function: rc522Auth
 * Description: verify card password
 * Input parameters:authMode--password verify mode
                    0x60 = verify A password key 
                    0x61 = verify B password key 
                    BlockAddr--Block address
                    Sectorkey--Block password
                    serNum--Card serial number ，4 bytes
 * Return: return MI_OK if successed
 */
int8_t rc522Auth(uint8_t authMode, uint8_t blockAddr, uint8_t* sectorKey, uint8_t* serNum) {
    int8_t status;
    uint16_t recvBits;
    uint8_t i;
    uint8_t buff[12]; 

    // Verify the command block address + sector + password + card serial number
    buff[0] = authMode;
    buff[1] = blockAddr;
    for (i = 0; i < 6; i++) 
        buff[i+2] = *(sectorKey + i);
    for (i = 0; i < 4; i++) 
        buff[i+8] = *(serNum + i);

    status = rc522ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);
    if ((status != MI_OK) || (!(rc522ReadRaw(RC522_REG_STATUS2) & 0x08)))
        status = MI_ERR;

    return status;
}

/*
 * Function: rc522ReadBlock
 * Description: Read data 
 * Input parameters:blockAddr--block address;recvData--the block data which are read
 * Return: return MI_OK if successed
 */
static int8_t rc522ReadBlock(uint8_t blockAddr, uint8_t* recvData) {
    int8_t status;
    uint16_t unLen;

    recvData[0] = PICC_READ;
    recvData[1] = blockAddr;
    rc522CalculateCRC(recvData,2, &recvData[2]);

    status = rc522ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);
    if ((status != MI_OK) || (unLen != 0x90))
        status = MI_ERR;

    return status;

}

/*
 * Function: rc522WriteBlock
 * Description: write block data
 * Input parameters:blockAddr--block address;writeData--Write 16 bytes data into block
 * Return: return MI_OK if successed
 */
int8_t rc522WriteBlock(uint8_t blockAddr, uint8_t* writeData) {
    int8_t status;
    uint16_t recvBits;
    uint8_t i;
    uint8_t buff[18]; 

    buff[0] = PICC_WRITE;
    buff[1] = blockAddr;
    rc522CalculateCRC(buff, 2, &buff[2]);
    status = rc522ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);
    if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
        status = MI_ERR;
    if (status == MI_OK) {
        // Data to the FIFO write 16Byte
        for (i = 0; i < 16; i++)
            buff[i] = *(writeData+i);

        rc522CalculateCRC(buff, 16, &buff[16]);

        status = rc522ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);
        if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)) 
            status = MI_ERR;
    }

    return status;

}

/*
 * Function: initRC522
 * Description: Initialize RC522
 * Input parameter: NULL
 * Return: NULL
 */
void initRC522(void) {
    unsigned char a;

//    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, LOW_PIN);
//    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1, HIGH_PIN);
    rc522Reset();
    rc522WriteRaw(RC522_REG_T_MODE, 0x8D);
    rc522WriteRaw(RC522_REG_T_PRESCALER, 0x3E);
    rc522WriteRaw(RC522_REG_T_RELOAD_L, 30);
    rc522WriteRaw(RC522_REG_T_RELOAD_H, 0);
    rc522WriteRaw(RC522_REG_RF_CFG, 0x70); // 48dB gain
    rc522WriteRaw(RC522_REG_TX_AUTO, 0x40);
    rc522WriteRaw(RC522_REG_MODE, 0x3D);
   a = rc522ReadRaw(RC522_REG_T_RELOAD_L);
   if(a != 30)
       DBG("rc522Init: No RC522 detected\r\n");
   else
       DBG("rc522Init: RC522 exist\r\n");
//    rc522AntennaOff();
    rc522AntennaOn(); // Open the antenna
}

/*
 * Function: rc522Reset
 * Description: reset RC522
 * Input parameter: NULL
 * Return: NULL
 */
void rc522Reset(void) {
    rc522WriteRaw(RC522_REG_COMMAND, PCD_RESETPHASE);
}

/*
 * Function: rc522AntennaOn
 * Description: Turn on antenna, every time turn on or shut down antenna need at least 1ms delay
 * Input parameter: NULL
 * Return: NULL
 */
static void rc522AntennaOn(void) {
    uint8_t temp;

    temp = rc522ReadRaw(RC522_REG_TX_CONTROL);
    if (!(temp & 0x03)) 
    rc522SetBitMask(RC522_REG_TX_CONTROL, 0x03);
}

/*
 * Function: rc522AntennaOff
 * Description: Turn off antenna, every time turn on or shut down antenna need at least 1ms delay
 * Input parameter: NULL
 * Return: NULL
 */
static void rc522AntennaOff(void) {
    rc522ClearBitMask(RC522_REG_TX_CONTROL, 0x03);
}

/*
 * Function: rc522Halt
 * Description: Command the cards into sleep mode
 * Input parameter: NULL
 * Return: NULL
 */
static void rc522Halt(void) {
    uint16_t unLen;
    uint8_t buff[4]; 

    buff[0] = PICC_HALT;
    buff[1] = 0;
    rc522CalculateCRC(buff, 2, &buff[2]);
    rc522ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);
}

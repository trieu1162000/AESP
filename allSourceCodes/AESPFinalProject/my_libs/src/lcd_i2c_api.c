/*
 * lcd_i2c_api.c
 *
 *  Created on: Oct 29, 2023
 *      Author: Trieu Huynh Pham Nhat
 */

#include "../inc/lcd_i2c_api.h"

static const char alpha_char[] = {
  0x08,
  0x14,
  0x1C,
  0x14,
  0x00,
  0x00,
  0x00,
  0x00
};

static const char root_char[] = {
  0x00,
  0x03,
  0x02,
  0x02,
  0x0A,
  0x1A,
  0x0A,
  0x04
};

#if 0
static void lcdFlush(struct lcd_i2c *lcd);
#endif

static void lcdHandleEscSeqChar(struct lcd_i2c *lcd, char ch);
static void lcdWrite(struct lcd_i2c *lcd, const char *buf, size_t count);

#if 0
static void mDelay(uint32_t ui32Second){
    SysCtlDelay(ui32Second * (SysCtlClockGet()/3000)); // Delay ms
}
#endif

static void uDelay(uint32_t ui32Second){
    SysCtlDelay(ui32Second * (SysCtlClockGet()/3000000)); // Delay us
}

static struct lcd_i2c_geometry lcd_i2c_geometry_20x4 = {
    .cols = 20,
    .rows = 4,
    .start_addrs = {0x00, 0x40, 0x14, 0x54},
};

static struct lcd_i2c_geometry lcd_i2c_geometry_16x2 = {
    .cols = 16,
    .rows = 2,
    .start_addrs = {0x00, 0x40},
};

static struct lcd_i2c_geometry lcd_i2c_geometry_8x1 = {
    .cols = 8,
    .rows = 1,
    .start_addrs = {0x00},
};

struct lcd_i2c_geometry *lcd_i2c_geometries[] = {
    &lcd_i2c_geometry_20x4,
    &lcd_i2c_geometry_16x2,
    &lcd_i2c_geometry_8x1,
    NULL
};

/* Defines possible register that we can write to */
typedef enum { IR, DR } dest_reg;

static void lcdRawWrite(struct lcd_i2c *lcd, uint8_t data)
{
    I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDRESS, false); // set address of slave for master to communicate with
    //put data to be sent into FIFO
    I2CMasterDataPut(I2C1_BASE, data);
    //Initiate send of data from the MCU
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_SEND);

    // Wait until MCU is done transferring.
    while(I2CMasterBusy(I2C1_BASE));
}

static void lcdWriteNibble(struct lcd_i2c *lcd, dest_reg reg, uint8_t data)
{
    /* Shift the interesting data on the upper 4 bits (b7-b4) */
    data = (data << 4) & 0xF0;

    /* Flip the RS bit if we write do data register */
    if (reg == DR)
        data |= RS;

    /* Keep the RW bit low, because we write */
    data = data | (RW & 0x00);

    /* Flip the backlight bit */
    if (lcd->backlight)
        data |= BL;

    lcdRawWrite(lcd, data);
    /* Theoretically wait for tAS = 40ns, practically it's already elapsed */

    /* Raise the E signal... */
    lcdRawWrite(lcd, data | E);
    /* Again, "wait" for pwEH = 230ns */

    /* ...and let it fall to clock the data into the HD44780's register */
    lcdRawWrite(lcd, data);
    /* And again, "wait" for about tCYC_E - pwEH = 270ns */
}

/*
 * Takes a regular 8-bit instruction and writes it's high nibble into device's
 * instruction register. The low nibble is assumed to be all zeros. This is
 * used with a physical 4-bit bus when the device is still expecting 8-bit
 * instructions.
 */
static void lcdWriteInstructionHighNibble(struct lcd_i2c *lcd, uint8_t data)
{
    uint8_t h = (data >> 4) & 0x0F;

    lcdWriteNibble(lcd, IR, h);

    uDelay(37);
}

static void lcdWriteInstruction(struct lcd_i2c *lcd, uint8_t data)
{
    uint8_t h = (data >> 4) & 0x0F;
    uint8_t l = data & 0x0F;

    lcdWriteNibble(lcd, IR, h);
    lcdWriteNibble(lcd, IR, l);

    uDelay(37);
}

static void lcdWriteData(struct lcd_i2c *lcd, uint8_t data)
{
    uint8_t h = (data >> 4) & 0x0F;
    uint8_t l = data & 0x0F;

    lcdWriteNibble(lcd, DR, h);
    lcdWriteNibble(lcd, DR, l);

    uDelay(37 + 4);
}

static void lcdWriteChar(struct lcd_i2c *lcd, char ch)
{
    struct lcd_i2c_geometry *geo = lcd->geometry;

    lcdWriteData(lcd, ch);

    lcd->pos.col++;

    if (lcd->pos.col == geo->cols) {
        lcd->pos.row = (lcd->pos.row + 1) % geo->rows;
        lcd->pos.col = 0;
        lcdWriteInstruction(lcd, LCD_I2C_DDRAM_ADDR | geo->start_addrs[lcd->pos.row]);
    }
}

void lcdClearLine(struct lcd_i2c *lcd)
{
    struct lcd_i2c_geometry *geo;
    int start_addr, col;

    geo = lcd->geometry;
    start_addr = geo->start_addrs[lcd->pos.row];

    lcdWriteInstruction(lcd, LCD_I2C_DDRAM_ADDR | start_addr);

    for (col = 0; col < geo->cols; col++)
        lcdWriteData(lcd, ' ');

    lcdWriteInstruction(lcd, LCD_I2C_DDRAM_ADDR | start_addr);
}

static void lcdHandleNewLine(struct lcd_i2c *lcd)
{
    struct lcd_i2c_geometry *geo = lcd->geometry;

    lcd->pos.row = (lcd->pos.row + 1) % geo->rows;
    lcd->pos.col = 0;
    lcdWriteInstruction(lcd, LCD_I2C_DDRAM_ADDR
                           | geo->start_addrs[lcd->pos.row]);
    lcdClearLine(lcd);
}

static void lcdHandleCarriageReturn(struct lcd_i2c *lcd)
{
    struct lcd_i2c_geometry *geo = lcd->geometry;

    lcd->pos.col = 0;
    lcdWriteInstruction(lcd, LCD_I2C_DDRAM_ADDR
                           | geo->start_addrs[lcd->pos.row]);
}

static void lcdLeaveEscSeq(struct lcd_i2c *lcd)
{
    memset(lcd->esc_seq_buf.buf, 0, ESC_SEQ_BUF_SIZE);
    lcd->esc_seq_buf.length = 0;
    lcd->is_in_esc_seq = false;
}

static lcdWriteRootChar(struct lcd_i2c *lcd, uint8_t data)
{
    struct lcd_i2c_geometry *geo = lcd->geometry;

    lcdWriteData(lcd, data);

    lcd->pos.col++;

    if (lcd->pos.col == geo->cols) {
        lcd->pos.row = (lcd->pos.row + 1) % geo->rows;
        lcd->pos.col = 0;
        lcdWriteInstruction(lcd, LCD_I2C_DDRAM_ADDR | geo->start_addrs[lcd->pos.row]);
    }

}

static void lcdWrite(struct lcd_i2c *lcd, const char *buf, size_t count)
{
    size_t i;
    char ch;

    if (lcd->dirty) {
        lcdClearDisplay(lcd);
        lcd->dirty = false;
    }

    for (i = 0UL; i < count; i++) {
        ch = buf[i];

        if (lcd->is_in_esc_seq) {
            lcdHandleEscSeqChar(lcd, ch);
        } else {
            switch (ch) {
            case '\r':
                lcdHandleCarriageReturn(lcd);
                break;
            case '\n':
                lcdHandleNewLine(lcd);
                break;
            case '\e':
                lcd->is_in_esc_seq = true;
                break;
            // This case handle for root character,
            // since we do not have any real root character
            case 'r':
                lcdWriteRootChar(lcd, LCD_ROOT_CHAR_ADDR);
                break;
            default:
                lcdWriteChar(lcd, ch);
                break;
            }
        }
    }
}

static void lcdFlushEscSeq(struct lcd_i2c *lcd)
{
    char *buf_to_flush;
    int buf_length;

    /* Copy and reset current esc seq */
    buf_to_flush = malloc(sizeof(char) * ESC_SEQ_BUF_SIZE);
    memcpy(buf_to_flush, lcd->esc_seq_buf.buf, ESC_SEQ_BUF_SIZE);
    buf_length = lcd->esc_seq_buf.length;

    lcdLeaveEscSeq(lcd);

    /* Write \e that initiated current esc seq */
    lcdWriteChar(lcd, '\e');

    /* Flush current esc seq */
    lcdWrite(lcd, buf_to_flush, buf_length);

    free(buf_to_flush);
}

static void lcdHandleEscSeqChar(struct lcd_i2c *lcd, char ch)
{
    int prev_row, prev_col;

    lcd->esc_seq_buf.buf[lcd->esc_seq_buf.length++] = ch;

    if (!strcmp(lcd->esc_seq_buf.buf, "[2J")) {
        prev_row = lcd->pos.row;
        prev_col = lcd->pos.col;

        lcdClearDisplay(lcd);
        lcdWriteInstruction(lcd, LCD_I2C_DDRAM_ADDR | (lcd->geometry->start_addrs[prev_row] + prev_col));

        lcdLeaveEscSeq(lcd);
    } else if (!strcmp(lcd->esc_seq_buf.buf, "[H")) {
        lcdWriteInstruction(lcd, LCD_I2C_RETURN_HOME);
        lcd->pos.row = 0;
        lcd->pos.col = 0;

        lcdLeaveEscSeq(lcd);
    } else if (lcd->esc_seq_buf.length == ESC_SEQ_BUF_SIZE) {
        lcdFlushEscSeq(lcd);
    }
}

// Create custom character for LCD to display
static void createCustomCharacter(struct lcd_i2c *lcd, const char cus_char_todo[8], uint8_t addr)
{
    int i = 0;

    lcdWriteInstruction(lcd, addr);
    for(; i < 8; i++)
        lcdWriteData(lcd, cus_char_todo[i]);
}

#if 0
static void lcdFlush(struct lcd_i2c *lcd)
{
    while (lcd->is_in_esc_seq)
        lcdFlushEscSeq(lcd);
}
#endif

void lcdClearDisplay(struct lcd_i2c *lcd)
{
    lcdWriteInstruction(lcd, LCD_I2C_CLEAR_DISPLAY);

    /* Wait for 1.64 ms because this one needs more time */
    uDelay(1640);

    /*
     * CLEAR_DISPLAY instruction also returns cursor to home,
     * so we need to update it locally.
     */
    lcd->pos.row = 0;
    lcd->pos.col = 0;
}

void lcdPrint(struct lcd_i2c *lcd, const char *str)
{
    lcdWrite(lcd, str, strlen(str));
}

static void lcdUpdateDisplayCtrl(struct lcd_i2c *lcd)
{

    lcdWriteInstruction(lcd, LCD_I2C_DISPLAY_CTRL
                           | LCD_I2C_D_DISPLAY_ON
                           | (lcd->cursor_display ? LCD_I2C_C_CURSOR_ON
                                                  : LCD_I2C_C_CURSOR_OFF)
                           | (lcd->cursor_blink ? LCD_I2C_B_BLINK_ON
                                                : LCD_I2C_B_BLINK_OFF));
}

#if 0
static void lcdSetGeometry(struct lcd_i2c *lcd, struct lcd_i2c_geometry *geo)
{
    lcd->geometry = geo;

    if (lcd->is_in_esc_seq)
        lcdLeaveEscSeq(lcd);

    lcdClearDisplay(lcd);
}
#endif

void lcdGotoXY(struct lcd_i2c *lcd, int row, int col)
{
    struct lcd_i2c_geometry *geo = lcd->geometry;
    lcd->pos.col = col;
    lcd->pos.row = row;

    lcdWriteInstruction(lcd, LCD_I2C_DDRAM_ADDR
                           | geo->start_addrs[lcd->pos.row]
                           | lcd->pos.col);

}

void lcdSetBackLight(struct lcd_i2c *lcd, bool backlight)
{
    lcd->backlight = backlight;
    lcdRawWrite(lcd, backlight ? BL : 0x00);
}

void lcdSetCursorBlink(struct lcd_i2c *lcd, bool cursor_blink)
{
    lcd->cursor_blink = cursor_blink;
    lcdUpdateDisplayCtrl(lcd);
}

void lcdSetCursorDisplay(struct lcd_i2c *lcd, bool cursor_display)
{
    lcd->cursor_display= cursor_display;
    lcdUpdateDisplayCtrl(lcd);
}

void lcdPrintAlphaCharacter(struct  lcd_i2c *lcd, uint8_t char_addr)
{
    lcdGotoXY(lcd, 1, 0);
    lcdWriteData(lcd, char_addr);
}

void initLCD(struct lcd_i2c *lcd)
{
    lcdWriteInstructionHighNibble(lcd, LCD_I2C_FUNCTION_SET
                                     | LCD_I2C_DL_8BITS);
    uDelay(4500); // wait min 4.1ms

    lcdWriteInstructionHighNibble(lcd, LCD_I2C_FUNCTION_SET
                                     | LCD_I2C_DL_8BITS);
    uDelay(4500); // wait min 4.1ms

    lcdWriteInstructionHighNibble(lcd, LCD_I2C_FUNCTION_SET
                                     | LCD_I2C_DL_8BITS);
    uDelay(150);


    lcdWriteInstructionHighNibble(lcd, LCD_I2C_FUNCTION_SET
                                     | LCD_I2C_DL_4BITS);

    lcdWriteInstruction(lcd, LCD_I2C_FUNCTION_SET
                           | LCD_I2C_DL_4BITS
                           | LCD_I2C_N_2LINES);

    lcdWriteInstruction(lcd, LCD_I2C_DISPLAY_CTRL
                           | LCD_I2C_D_DISPLAY_ON
                           | LCD_I2C_C_CURSOR_ON
                           | LCD_I2C_B_BLINK_ON);

    lcdClearDisplay(lcd);

    lcdWriteInstruction(lcd, LCD_I2C_ENTRY_MODE_SET
                           | LCD_I2C_ID_INCREMENT
                           | LCD_I2C_S_SHIFT_OFF);
}

void initParamsLCD(struct lcd_i2c *lcd, struct lcd_i2c_geometry *geometry)
{
    lcd->geometry = geometry;
    lcd->pos.row = 0;
    lcd->pos.col = 0;
    memset(lcd->esc_seq_buf.buf, 0, ESC_SEQ_BUF_SIZE);
    lcd->esc_seq_buf.length = 0;
    lcd->is_in_esc_seq = false;
    lcd->backlight = true;
    lcd->cursor_blink = false;
    lcd->cursor_display = false;
    createCustomCharacter(lcd, alpha_char, LCD_I2C_CGRAM_ADDR);         // alpha character
    createCustomCharacter(lcd, root_char, LCD_I2C_CGRAM_ADDR + 56);      // root character
}

/*
 * lcd_i2c_api.h
 *
 *  Created on: Oct 29, 2023
 *      Author: Trieu Huynh Pham Nhat
 */

#include "config_peripherals_api.h"
#include "driverlib/i2c.h"

#ifndef MY_LIBS_INC_LCD_I2C_API_H_
#define MY_LIBS_INC_LCD_I2C_API_H_

#define BL    0x08
#define E     0x04
#define RW    0x02
#define RS    0x01

#define LCD_I2C_CLEAR_DISPLAY       0x01
#define LCD_I2C_RETURN_HOME         0x02
#define LCD_I2C_ENTRY_MODE_SET      0x04
#define LCD_I2C_DISPLAY_CTRL        0x08
#define LCD_I2C_SHIFT               0x10
#define LCD_I2C_FUNCTION_SET        0x20
#define LCD_I2C_CGRAM_ADDR          0x40
#define LCD_I2C_DDRAM_ADDR          0x80

#define LCD_I2C_DL_8BITS            0x10
#define LCD_I2C_DL_4BITS            0x00
#define LCD_I2C_N_2LINES            0x08
#define LCD_I2C_N_1LINE             0x00

#define LCD_I2C_D_DISPLAY_ON        0x04
#define LCD_I2C_D_DISPLAY_OFF       0x00
#define LCD_I2C_C_CURSOR_ON         0x02
#define LCD_I2C_C_CURSOR_OFF        0x00
#define LCD_I2C_B_BLINK_ON          0x01
#define LCD_I2C_B_BLINK_OFF         0x00

#define LCD_I2C_ID_INCREMENT        0x02
#define LCD_I2C_ID_DECREMENT        0x00
#define LCD_I2C_S_SHIFT_ON          0x01
#define LCD_I2C_S_SHIFT_OFF         0x00

#define I2C_HANDLER                 I2C1_BASE
#define SLAVE_ADDRESS               0x27
#define BUF_SIZE                    64
#define ESC_SEQ_BUF_SIZE            4

#define LCD_ALPHA_CHAR_ADDR         0U
#define LCD_ROOT_CHAR_ADDR          7U

struct lcd_i2c_geometry {
    int cols;
    int rows;
    int start_addrs[];
};

/* just a struct describing for the LCD */
struct lcd_i2c {
    struct lcd_i2c_geometry *geometry;

    /* Current cursor position on the display */
    struct {
        int row;
        int col;
    } pos;

    char buf[BUF_SIZE];
    struct {
        char buf[ESC_SEQ_BUF_SIZE];
        int length;
    } esc_seq_buf;
    bool is_in_esc_seq;

    bool backlight;
    bool cursor_blink;
    bool cursor_display;

    bool dirty;
};

extern void lcdPrint(struct lcd_i2c *, const char *);
extern void lcdSetBackLight(struct lcd_i2c *, bool);
extern void lcdSetCursorBlink(struct lcd_i2c *, bool);
extern void lcdSetCursorDisplay(struct lcd_i2c *, bool);
extern void initLCD(struct lcd_i2c *);
extern void initParamsLCD(struct lcd_i2c *, struct lcd_i2c_geometry *);
extern void lcdClearDisplay(struct lcd_i2c *);
extern void lcdGotoXY(struct lcd_i2c *, int, int);
extern void lcdPrintAlphaCharacter(struct  lcd_i2c *, uint8_t);
extern void lcdClearLine(struct lcd_i2c *);

extern struct lcd_i2c_geometry *lcd_i2c_geometries[];
extern struct lcd_i2c *lcd;

#endif /* MY_LIBS_INC_LCD_I2C_API_H_ */

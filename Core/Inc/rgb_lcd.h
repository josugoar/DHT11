#ifndef __RGB_LCD_H__
#define __RGB_LCD_H__

#include <inttypes.h>
#include "main.h"

// Device I2C Arress
#define LCD_ADDRESS     (0x7c>>1)
#define RGB_ADDRESS     (0xc4>>1)
#define RGB_ADDRESS_V5  (0x30)


// color define
#define WHITE           0
#define RED             1
#define GREEN           2
#define BLUE            3

#define REG_MODE1       0x00
#define REG_MODE2       0x01
#define REG_OUTPUT      0x08

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

typedef struct rgb_lcd {

    uint8_t rgb_chip_addr;

    uint8_t _displayfunction;
    uint8_t _displaycontrol;
    uint8_t _displaymode;

    uint8_t _initialized;

    uint8_t _numlines, _currline;

    I2C_HandleTypeDef *_wire;
} rgb_lcd;

void rgb_lcd_init(rgb_lcd *_this);

void rgb_lcd_begin(rgb_lcd *_this, uint8_t cols, uint8_t rows, uint8_t charsize, I2C_HandleTypeDef *wire);

void rgb_lcd_clear(rgb_lcd *_this);
void rgb_lcd_home(rgb_lcd *_this);

void rgb_lcd_noDisplay(rgb_lcd *_this);
void rgb_lcd_display(rgb_lcd *_this);
void rgb_lcd_noBlink(rgb_lcd *_this);
void rgb_lcd_blink(rgb_lcd *_this);
void rgb_lcd_noCursor(rgb_lcd *_this);
void rgb_lcd_cursor(rgb_lcd *_this);
void rgb_lcd_scrollDisplayLeft(rgb_lcd *_this);
void rgb_lcd_scrollDisplayRight(rgb_lcd *_this);
void rgb_lcd_leftToRight(rgb_lcd *_this);
void rgb_lcd_rightToLeft(rgb_lcd *_this);
void rgb_lcd_autoscroll(rgb_lcd *_this);
void rgb_lcd_noAutoscroll(rgb_lcd *_this);

void rgb_lcd_createChar(rgb_lcd *_this, uint8_t, uint8_t[]);
void rgb_lcd_setCursor(rgb_lcd *_this, uint8_t, uint8_t);

size_t rgb_lcd_write(rgb_lcd *_this, uint8_t);
void rgb_lcd_print(rgb_lcd* _this, uint8_t *value, uint32_t len);
void rgb_lcd_command(rgb_lcd *_this, uint8_t);

// color control
void rgb_lcd_setRGB(rgb_lcd *_this, unsigned char r, unsigned char g, unsigned char b); // set rgb
void rgb_lcd_setPWM(rgb_lcd *_this, unsigned char color, unsigned char pwm); // set pwm

void rgb_lcd_setColor(rgb_lcd *_this, unsigned char color);
static void rgb_lcd_setColorAll(rgb_lcd *_this) {
    rgb_lcd_setRGB(_this, 0, 0, 0);
}
static void rgb_lcd_setColorWhite(rgb_lcd *_this) {
    rgb_lcd_setRGB(_this, 255, 255, 255);
}

// blink the LED backlight
void rgb_lcd_blinkLED(rgb_lcd *_this);
void rgb_lcd_noBlinkLED(rgb_lcd *_this);

#endif

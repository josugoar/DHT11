#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "rgb_lcd.h"
#include "wiring_time.h"

#define TRIALS 1
#define TIMEOUT 25

void rgb_lcd_send(rgb_lcd* _this, uint8_t, uint8_t);
void rgb_lcd_setReg(rgb_lcd* _this, unsigned char addr, unsigned char dta);
void rgb_lcd_i2c_send_byteS(rgb_lcd* _this, unsigned char* dta, unsigned char len);

void rgb_lcd_i2c_send_byteS(rgb_lcd* _this, unsigned char* dta, unsigned char len) {
    HAL_I2C_Master_Transmit(_this->_wire, LCD_ADDRESS << 1, dta, len, TIMEOUT);
}

void rgb_lcd_init(rgb_lcd* _this) {
    _this->_displayfunction = 0;
    _this->_displaycontrol = 0;
    _this->_displaymode = 0;
    _this->_initialized = 0;
    _this->_numlines = 0;
    _this->_currline = 0;
}

void rgb_lcd_begin(rgb_lcd* _this, uint8_t cols, uint8_t lines, uint8_t dotsize, I2C_HandleTypeDef *wire) {

    _this->_wire = wire;

    if (lines > 1) {
        _this->_displayfunction |= LCD_2LINE;
    }
    _this->_numlines = lines;
    _this->_currline = 0;

    // for some 1 line displays you can select a 10 pixel high font
    if ((dotsize != 0) && (lines == 1)) {
        _this->_displayfunction |= LCD_5x10DOTS;
    }

    // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
    // according to datasheet, we need at least 40ms after power rises above 2.7V
    // before sending commands. MCU can turn on way befer 4.5V so we'll wait 50
    delayMicroseconds(50000);


    // this is according to the hitachi HD44780 datasheet
    // page 45 figure 23

    // Send function set command sequence
    rgb_lcd_command(_this, LCD_FUNCTIONSET | _this->_displayfunction);
    delayMicroseconds(4500);  // wait more than 4.1ms

    // second try
    rgb_lcd_command(_this, LCD_FUNCTIONSET | _this->_displayfunction);
    delayMicroseconds(150);

    // third go
    rgb_lcd_command(_this, LCD_FUNCTIONSET | _this->_displayfunction);


    // finally, set # lines, font size, etc.
    rgb_lcd_command(_this, LCD_FUNCTIONSET | _this->_displayfunction);

    // turn the display on with no cursor or blinking default
    _this->_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    rgb_lcd_display(_this);

    // clear it off
    rgb_lcd_clear(_this);

    // Initialize to default text direction (for romance languages)
    _this->_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    // set the entry mode
    rgb_lcd_command(_this, LCD_ENTRYMODESET | _this->_displaymode);


    // check rgb chip model
    if (HAL_I2C_IsDeviceReady(_this->_wire, RGB_ADDRESS_V5 << 1, TRIALS, TIMEOUT) == 0)
    {
        _this->rgb_chip_addr = RGB_ADDRESS_V5;
        rgb_lcd_setReg(_this, 0x00, 0x07); // reset the chip
        delayMicroseconds(200); // wait 200 us to complete
        rgb_lcd_setReg(_this, 0x04, 0x15); // set all led always on
    }
    else
    {
        _this->rgb_chip_addr = RGB_ADDRESS;
        // backlight init
        rgb_lcd_setReg(_this, REG_MODE1, 0);
        // set LEDs controllable by both PWM and GRPPWM registers
        rgb_lcd_setReg(_this, REG_OUTPUT, 0xFF);
        // set MODE2 values
        // 0010 0000 -> 0x20  (DMBLNK to 1, ie blinky mode)
        rgb_lcd_setReg(_this, REG_MODE2, 0x20);
    }

    rgb_lcd_setColorWhite(_this);

}

/********** high level commands, for the user! */
void rgb_lcd_clear(rgb_lcd* _this) {
    rgb_lcd_command(_this, LCD_CLEARDISPLAY);        // clear display, set cursor position to zero
    delayMicroseconds(2000);          // this command takes a long time!
}

void rgb_lcd_home(rgb_lcd* _this) {
    rgb_lcd_command(_this, LCD_RETURNHOME);        // set cursor position to zero
    delayMicroseconds(2000);        // this command takes a long time!
}

void rgb_lcd_setCursor(rgb_lcd* _this, uint8_t col, uint8_t row) {

    col = (row == 0 ? col | 0x80 : col | 0xc0);
    unsigned char dta[2] = {0x80, col};

    rgb_lcd_i2c_send_byteS(_this, dta, 2);

}

// Turn the display on/off (quickly)
void rgb_lcd_noDisplay(rgb_lcd* _this) {
    _this->_displaycontrol &= ~LCD_DISPLAYON;
    rgb_lcd_command(_this, LCD_DISPLAYCONTROL | _this->_displaycontrol);
}

void rgb_lcd_display(rgb_lcd* _this) {
    _this->_displaycontrol |= LCD_DISPLAYON;
    rgb_lcd_command(_this, LCD_DISPLAYCONTROL | _this->_displaycontrol);
}

// Turns the underline cursor on/off
void rgb_lcd_noCursor(rgb_lcd* _this) {
    _this->_displaycontrol &= ~LCD_CURSORON;
    rgb_lcd_command(_this, LCD_DISPLAYCONTROL | _this->_displaycontrol);
}

void rgb_lcd_cursor(rgb_lcd* _this) {
    _this->_displaycontrol |= LCD_CURSORON;
    rgb_lcd_command(_this, LCD_DISPLAYCONTROL | _this->_displaycontrol);
}

// Turn on and off the blinking cursor
void rgb_lcd_noBlink(rgb_lcd* _this) {
    _this->_displaycontrol &= ~LCD_BLINKON;
    rgb_lcd_command(_this, LCD_DISPLAYCONTROL | _this->_displaycontrol);
}
void rgb_lcd_blink(rgb_lcd* _this) {
    _this->_displaycontrol |= LCD_BLINKON;
    rgb_lcd_command(_this, LCD_DISPLAYCONTROL | _this->_displaycontrol);
}

// These commands scroll the display without changing the RAM
void rgb_lcd_scrollDisplayLeft(rgb_lcd* _this) {
    rgb_lcd_command(_this, LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void rgb_lcd_scrollDisplayRight(rgb_lcd* _this) {
    rgb_lcd_command(_this, LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void rgb_lcd_leftToRight(rgb_lcd* _this) {
    _this->_displaymode |= LCD_ENTRYLEFT;
    rgb_lcd_command(_this, LCD_ENTRYMODESET | _this->_displaymode);
}

// This is for text that flows Right to Left
void rgb_lcd_rightToLeft(rgb_lcd* _this) {
    _this->_displaymode &= ~LCD_ENTRYLEFT;
    rgb_lcd_command(_this, LCD_ENTRYMODESET | _this->_displaymode);
}

// This will 'right justify' text from the cursor
void rgb_lcd_autoscroll(rgb_lcd* _this) {
    _this->_displaymode |= LCD_ENTRYSHIFTINCREMENT;
    rgb_lcd_command(_this, LCD_ENTRYMODESET | _this->_displaymode);
}

// This will 'left justify' text from the cursor
void rgb_lcd_noAutoscroll(rgb_lcd* _this) {
    _this->_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
    rgb_lcd_command(_this, LCD_ENTRYMODESET | _this->_displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void rgb_lcd_createChar(rgb_lcd* _this, uint8_t location, uint8_t charmap[]) {

    location &= 0x7; // we only have 8 locations 0-7
    rgb_lcd_command(_this, LCD_SETCGRAMADDR | (location << 3));


    unsigned char dta[9];
    dta[0] = 0x40;
    for (int i = 0; i < 8; i++) {
        dta[i + 1] = charmap[i];
    }
    rgb_lcd_i2c_send_byteS(_this, dta, 9);
}

// Control the backlight LED blinking
void rgb_lcd_blinkLED(rgb_lcd* _this) {
    if (_this->rgb_chip_addr == RGB_ADDRESS_V5)
    {
        // attach all led to pwm1
        // blink period in seconds = (<reg 1> + 2) *0.128s
        // pwm1 on/off ratio = <reg 2> / 256
        rgb_lcd_setReg(_this, 0x04, 0x2a);  // 0010 1010
        rgb_lcd_setReg(_this, 0x01, 0x06);  // blink every second
        rgb_lcd_setReg(_this, 0x02, 0x7f);  // half on, half off
    }
    else
    {
        // blink period in seconds = (<reg 7> + 1) / 24
        // on/off ratio = <reg 6> / 256
        rgb_lcd_setReg(_this, 0x07, 0x17);  // blink every second
        rgb_lcd_setReg(_this, 0x06, 0x7f);  // half on, half off
    }


}

void rgb_lcd_noBlinkLED(rgb_lcd* _this) {
    if (_this->rgb_chip_addr == RGB_ADDRESS_V5)
    {
        rgb_lcd_setReg(_this, 0x04, 0x15);  // 0001 0101
    }
    else
    {
        rgb_lcd_setReg(_this, 0x07, 0x00);
        rgb_lcd_setReg(_this, 0x06, 0xff);
    }
}

/*********** mid level commands, for sending data/cmds */

// send command
inline void rgb_lcd_command(rgb_lcd* _this, uint8_t value) {
    unsigned char dta[2] = {0x80, value};
    rgb_lcd_i2c_send_byteS(_this, dta, 2);
}

// send data
inline size_t rgb_lcd_write(rgb_lcd* _this, uint8_t value) {

    unsigned char dta[2] = {0x40, value};
    rgb_lcd_i2c_send_byteS(_this, dta, 2);
    return 1; // assume sucess
}

inline void rgb_lcd_print(rgb_lcd* _this, uint8_t *value, uint32_t len) {
    for (size_t i = 0; i < len; i++)
    {
        rgb_lcd_write(_this, value[i]);
    }
}

void rgb_lcd_setReg(rgb_lcd* _this, unsigned char reg, unsigned char dat) {
    unsigned char dta[2] = {reg, dat};
    HAL_I2C_Master_Transmit(_this->_wire, _this->rgb_chip_addr << 1, dta, 2, TIMEOUT);
}

void rgb_lcd_setRGB(rgb_lcd* _this, unsigned char r, unsigned char g, unsigned char b) {
    if (_this->rgb_chip_addr == RGB_ADDRESS_V5)
    {
        rgb_lcd_setReg(_this, 0x06, r);
        rgb_lcd_setReg(_this, 0x07, g);
        rgb_lcd_setReg(_this, 0x08, b);
    }
    else
    {
        rgb_lcd_setReg(_this, 0x04, r);
        rgb_lcd_setReg(_this, 0x03, g);
        rgb_lcd_setReg(_this, 0x02, b);
    }
}

void rgb_lcd_setPWM(rgb_lcd* _this, unsigned char color, unsigned char pwm) {
    switch (color)
    {
        case WHITE:
            rgb_lcd_setRGB(_this, pwm, pwm, pwm);
            break;
        case RED:
            rgb_lcd_setRGB(_this, pwm, 0, 0);
            break;
        case GREEN:
            rgb_lcd_setRGB(_this, 0, pwm, 0);
            break;
        case BLUE:
            rgb_lcd_setRGB(_this, 0, 0, pwm);
            break;
        default:
            break;
    }
}

const unsigned char color_define[4][3] = {
    {255, 255, 255},            // white
    {255, 0, 0},                // red
    {0, 255, 0},                // green
    {0, 0, 255},                // blue
};

void rgb_lcd_setColor(rgb_lcd* _this, unsigned char color) {
    if (color > 3) {
        return ;
    }
    rgb_lcd_setRGB(_this, color_define[color][0], color_define[color][1], color_define[color][2]);
}

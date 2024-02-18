#include "lcd1602.h"
#include "main.h"

#define PULSE_PERIOD 1 // 0.5ms
#define CMD_PERIOD 4 // 4.1ms

#define BACKLIGHT 8
#define DATA 1
static int iBackLight = BACKLIGHT;

extern I2C_HandleTypeDef hi2c1;

static void WriteCommand(uint8_t ucCMD) {
    uint8_t uc;
    HAL_StatusTypeDef res;

    uc = (ucCMD & 0xf0) | iBackLight; // most significant nibble sent first
    res = HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(LCD_ADDRESS << 1), &uc, 1, HAL_MAX_DELAY);
    HAL_Delay(PULSE_PERIOD); // manually pulse the clock line
    uc |= 4; // enable pulse
    res = HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(LCD_ADDRESS << 1), &uc, 1, HAL_MAX_DELAY);
    HAL_Delay(PULSE_PERIOD);
    uc &= ~4; // toggle pulse
    res = HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(LCD_ADDRESS << 1), &uc, 1, HAL_MAX_DELAY);
    HAL_Delay(CMD_PERIOD);
    uc = iBackLight | (ucCMD << 4); // least significant nibble
    res = HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(LCD_ADDRESS << 1), &uc, 1, HAL_MAX_DELAY);
    HAL_Delay(PULSE_PERIOD);
    uc |= 4; // enable pulse
    res = HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(LCD_ADDRESS << 1), &uc, 1, HAL_MAX_DELAY);
    HAL_Delay(PULSE_PERIOD);
    uc &= ~4; // toggle pulse
    res = HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(LCD_ADDRESS << 1), &uc, 1, HAL_MAX_DELAY);
    HAL_Delay(CMD_PERIOD);
} /* WriteCommand() */

//
// Control the backlight, cursor, and blink
// The cursor is an underline and is separate and distinct
// from the blinking block option
//
int lcd1602Control(int bBacklight, int bCursor, int bBlink) {
    unsigned char ucCMD = 0x0C; // Display control command

    iBackLight = (bBacklight) ? BACKLIGHT : 0;
    if (bCursor) ucCMD |= 0x02;
    if (bBlink) ucCMD |= 0x01;
    WriteCommand(ucCMD);

    return 0;
} /* lcd1602Control() */

//
// Write an ASCII string (up to 16 characters at a time)
//
int lcd1602WriteString(char *text) {
    unsigned char ucTemp[2];
    int i = 0;

    if (text == NULL) return 1;

    while (i < 16 && *text) {
        ucTemp[0] = iBackLight | DATA | (*text & 0xF0);
        HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(LCD_ADDRESS << 1), ucTemp, 1, HAL_MAX_DELAY);
        HAL_Delay(PULSE_PERIOD);
        ucTemp[0] |= 0x04; // Pulse E
        HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(LCD_ADDRESS << 1), ucTemp, 1, HAL_MAX_DELAY);
        HAL_Delay(PULSE_PERIOD);
        ucTemp[0] &= ~0x04;
        HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(LCD_ADDRESS << 1), ucTemp, 1, HAL_MAX_DELAY);
        HAL_Delay(PULSE_PERIOD);

        ucTemp[0] = iBackLight | DATA | (*text << 4);
        HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(LCD_ADDRESS << 1), ucTemp, 1, HAL_MAX_DELAY);
        ucTemp[0] |= 0x04; // Pulse E
        HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(LCD_ADDRESS << 1), ucTemp, 1, HAL_MAX_DELAY);
        HAL_Delay(PULSE_PERIOD);
        ucTemp[0] &= ~0x04;
        HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(LCD_ADDRESS << 1), ucTemp, 1, HAL_MAX_DELAY);
        HAL_Delay(CMD_PERIOD);

        text++;
        i++;
    }
    return 0;
} /* lcd1602WriteString() */

//
// Erase the display memory and reset the cursor to 0,0
//
int lcd1602Clear(void) {
    WriteCommand(0x01); // Clear display command
    HAL_Delay(CMD_PERIOD); // Wait for the command to execute
    return 0;
} /* lcd1602Clear() */

//
// Initialize the LCD
// This includes setting up the I2C, configuring the LCD,
// and clearing the display
//
int lcd1602Init(void) {
    iBackLight = BACKLIGHT; // Turn on backlight
    WriteCommand(0x02); // Set 4-bit mode of the LCD controller
    WriteCommand(0x28); // 2 lines, 5x8 dot matrix
    WriteCommand(0x0C); // Display on, cursor off
    WriteCommand(0x06); // Inc cursor to right when writing and don't scroll
    WriteCommand(0x80); // Set cursor to row 1, column 1
    lcd1602Clear(); // Clear the display
    return 0;
} /* lcd1602Init() */

//
// Set the LCD cursor position
//
int lcd1602SetCursor(int x, int y) {
    unsigned char cmd;

    if (x < 0 || x > 15 || y < 0 || y > 1) return 1;

    cmd = (y == 0) ? 0x80 : 0xC0; // Set the correct line
    cmd |= x;
    WriteCommand(cmd);
    return 0;
} /* lcd1602SetCursor() */

void lcd1602Shutdown(void) {
    iBackLight = 0; // Turn off backlight
    WriteCommand(0x08); // Turn off display, cursor, and blink
} /* lcd1602Shutdown() */

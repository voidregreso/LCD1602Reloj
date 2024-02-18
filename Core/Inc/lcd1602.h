#include "stm32f4xx_hal.h" 

#define LCD_ADDRESS 0x27

int lcd1602Init(void);
int lcd1602SetCursor(int x, int y);
void lcd1602Shutdown(void);
int lcd1602Clear(void);
int lcd1602WriteString(char *text);
int lcd1602Control(int bBacklight, int bCursor, int bBlink);

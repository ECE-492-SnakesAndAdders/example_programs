#ifndef LCD_H
#define LCD_H

#include <SystemClock.h>
#include <XPD.h>
#include <GPIO.h>
#include <Thread.h>
#include <SPI.h>
//#include "main.h"
#include "time_funcs.h"
namespace LCD_Functions {

 enum class DisplaySettings : uint16_t {
    // Show Cursor, Blink, Display disable
    DISPLAY_ON = 0x04,
    DISPLAY_OFF = 0x03
 };
 enum class CursorSettings : uint16_t {
    CURSOR_ON = 0x02,
    CURSOR_OFF = 0x05,
 };
 enum class CursorBlink : uint16_t {
    BLINK_ON = 0x01,
    BLINK_OFF = 0x06
};
enum class Cursor_Line : uint16_t {
    LCD_HOME_L1 = 0x80,
    LINE1 = 0,
    LINE2 = 0x20,
    LINE3 = 0x40,
    LINE4 = 0x60

};
 /*enum class Character_Sets : uint16_t{
        // Different character sets can be selected
        ROM_PAGE_EUROPE = 0x00,
        ROM_PAGE_CYRILLIC = 0x04,
        ROM_PAGE_JAPANESE = 0x0C
};*/
}
template<LCD_Functions :: DisplaySettings DISPLAYMODE, LCD_Functions :: CursorSettings CURSORMODE, LCD_Functions ::CursorBlink BlinkMode, LCD_Functions :: Cursor_Line CURSORLINE>
class LCD_STARTUP {
    public:
    // Settings of display, cursor and blinking via flags
void setDisplayMode()
{
    writeIns(0x08 + (uint16_t)DISPLAYMODE);
}

void setCursorMode(){
    writeIns(0x08 + (uint16_t)CURSORMODE);
}
void setBlinkMode(){ // Setting Blink Mode
    writeIns(0x08 + (uint16_t)BlinkMode);
}
void setCursor(uint16_t col, uint16_t row)
{
    int row_offsets[] = {0x00, 0x20, 0x40, 0x60};
    if (row > 3)
    {
        row = 3; // we count rows starting w/0
    }
    if (col > 19)
    {
        row = 19; // we count cols starting w/0
    }
    writeIns((uint16_t)CURSORLINE + col + row_offsets[row]);
}
void initializeDOGM204() // Initialization of the EA DOGM204, as in the specs.
{
    // init display, copied directly from the dogm204e.pdf
    writeIns(0x3A); // 8-Bit data length extension Bit RE=1; REV=0
    writeIns(0x09); // 4 line display
    writeIns(0x06); // Bottom view
    writeIns(0x1E); // Bias setting BS1=1
    writeIns(0x39); // 8-Bit data length extension Bit RE=0; IS=1
    writeIns(0x1B); // BS0=1 -> Bias=1/6
    writeIns(0x6E); // Divider on and set value
    writeIns(0x57); // Booster on and set contrast (BB1=C5, DB0=C4)
    writeIns(0x72); // Set contrast (DB3-DB0=C3-C0)
    writeIns(0x38); // 8-Bit data length extension Bit RE=0; IS=0
    writeIns(0x0F); // Display on, cursor on, blink on
}
void setViewAngleBottom()// Direction of view when LED connections are at the bottom
{
    writeIns(0x3A);
    writeIns(0x05);
    writeIns(0x38);
}
void setViewAngleTop()// Direction of view when LED connections are at the bottom
{
    writeIns(0x3A);
    writeIns(0x06);
    writeIns(0x38);
}
void setContrast(uint16_t p_contr) // set contrast, goes from 0 to 63, 63 is maximum
{
    writeIns(0x39);
    writeIns(0x54 | (p_contr >> 4));
    writeIns(0x70 | (p_contr & 0x0F));
    writeIns(0x38);
}
void clrDisplay() // Clear the LCD
{
    writeIns(0x01);
    setCursor(0, 0);
    wait_ms(100); // Wait 100 milliseconds
}
void writeChar(uint16_t p_x, uint16_t p_y, uint16_t p_char)
{
    setCursor(p_x, p_y);
    writeData(p_char);
}
void writeString(uint16_t p_x, uint16_t p_y, uint16_t p_str[], int len)
{
    int idx = 0;
    while (idx < len)
    {
        setCursor(p_x + idx, p_y);
        writeChar(p_str[idx]);
        idx = idx + 1;
    }
}
void writeChar(uint16_t p_char)
{
    writeData(p_char);
}
void takeInput(uint16_t myString)
{
    // int len = sizeof(myString) / sizeof(uint16_t);
    writeChar(myString);
}
private: 
void writeIns(uint16_t insCode)
{
    SPI_write_LSB(0x1F, SPI0);                    // Send 5 synchronisation bits, RS = 0, R/W = 0
    SPI_write_LSB((insCode & 0x0F), SPI0);        // send lower data bits
    SPI_write_LSB(((insCode >> 4) & 0x0F), SPI0); // send higher data bits
}
void SPI_write_LSB(uint16_t val, enum SPI_name name)
{
    val = sfu_reverse_bits(val);
    val = (val >> 8);
    uint16_t out_reg = get_spi_addr(SPI_TX, name);
    uint16_t in_reg = get_spi_addr(SPI_RX, name);
    _outp(val, out_reg);
    _inp(in_reg);
}
void writeData(uint16_t data)
{
    SPI_write_LSB(0x5F, SPI0);                 // Send 5 synchronisation bits, RS = 1, R/W = 0
    SPI_write_LSB((data & 0x0F), SPI0);        // send lower data bits
    SPI_write_LSB(((data >> 4) & 0x0F), SPI0); // send higher data bits
}

};

// Reset the LCD to its original state, once briefly to low at the RESET PIN of the LCD
void ResetLCD()
{
    gpio_write(gpio_get_output_reg(GPIO_C) & ~(1 << 3), GPIO_C);
    wait_ms(500);
    gpio_write(gpio_get_output_reg(GPIO_C) | (1 << 3), GPIO_C);
    wait_ms(100);
}
// Select character set, with constants defined above
/*void setROMPAGE(unsigned uint16_t p_rompage)
{
    writeIns(0x2A);
    writeIns(0x72);
    writeData(p_rompage);
    writeIns(0x28);
}*/


#endif // _LCD_H_
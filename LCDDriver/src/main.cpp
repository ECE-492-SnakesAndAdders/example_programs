///
/// Author: Devin Headrick
/// Desc: Simple program to demonstrate LCD Screen

#include "time_funcs.h"
#include "SimpleUart.h"
// #include <Structs.h>
#include <Thread.h>
#include <XPD.h>
#include "LCD.h"

using namespace LCD_Functions;
using L = LCD_STARTUP<DisplaySettings::DISPLAY_ON, CursorSettings ::CURSOR_ON, CursorBlink ::BLINK_OFF, Cursor_Line ::LCD_HOME_L1>;
L LCD_startup;


// sys_clock_wait can only wait a maximum of 65535 ticks
// use a loop to get a longer delay.
void long_wait()
{
  for (int i = 0; i < 10000; ++i) {
    sys_clock_wait(10000);
  }
}

int main(void)
{   
     // init_clock();
     /// This sets the crystal freq again? Even tho its already set in the init.cpp file
     /// Going to ignore this for now 

     SPI_set_config((SPI_ENABLE | SPI_MASTER | SPI_CLK_RATE_DIV_256) & ~(SPI_CLK_PHASE | SPI_CLK_IDLE_AT_1), SPI0);
    
     // This sets pins PC2 and PC3 Data Direction Register (DD2 , DD3) to 1, which makes them outputs.
     // This is via shifting 0000 1100 to MSB of uint16 input of gpio_set_config. => 0000 1100 0000 0000 
     gpio_set_config((0x0C << 8), GPIO_C); // PC3 will be used as the Reset Pin, PC2 is CS 

     //writes following pins 0010 1100 
     // gpio_write(gpio_get_output_reg(GPIO_C) | 0xC, GPIO_C);
     gpio_write(gpio_get_output_reg(GPIO_C) | 0xC, GPIO_C);

     ResetLCD();
     LCD_startup.initializeDOGM204();
     LCD_startup.setViewAngleTop();
     LCD_startup.clrDisplay();
     LCD_startup.setDisplayMode();
     LCD_startup.setCursorMode();
     LCD_startup.setBlinkMode();
     LCD_startup.setCursor(0, 0);
     // uint16_t myString[] = {'A', 'D', 'E', 'E', 'B'};
     // int len = sizeof(myString) / sizeof(uint16_t);
     // LCD_startup.writeString(4, 0, myString, len);
     //   setROMPAGE(ROM_PAGE_CYRILLIC); //Already commented out 

     char* xpd_test_str = "testing";

     //Have the board loop through printing various ascii characters 
     // Range from d33 -> d126  ( ! -> ~)
     char ch = 33; //ASCII A

     while(true){
          if(ch > 126){
               ch = 33;
          }
          LCD_startup.writeChar(ch);
          long_wait();
          ch++;

          // xpd_puts(xpd_test_str);
          // long_wait();
     }
}
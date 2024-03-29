#define BOARD_C3
#include "time_funcs.h"
// #include "UART.h"

#include <Structs.h>
#include <Thread.h>
#include <XPD.h>
#include "GPIO_nio.h"
#include "UART_GPIO_NIO.h"

int main(void)
{
  uint16_t count = 0;
  UART UART;
  GPIO TX;
  GPIO RX;
  TX.init(PORT_C, 6, OUT);
  RX.init(PORT_C, 7, IN);

  //Setup UART driver at 96 baud , 8 databits (a byte), 0 parity, 0 ptype , 1 STOP bit
  UART.initTX(&TX, 96, 8, 0, 0, 1);
  UART.initRX(&RX, 96, 8, 0, 0, 1);

//FOR TESTING TX BYTE
  // while(true){
  //   for (int i = 0; i < (sizeof(test_data) / sizeof(test_data[0])); i++){
  //     UART.transmitByte(test_data[i]);
  //   }
  //   wait_ms(1000);
  // }


//FOR TESTING ECHO SEND/ RCV CHARS IN SERIAL TERMINAL 
  // uint16_t test_byte = 0;
  //  while(true){
  //   test_byte = UART.readByte();
  //   if(test_byte != 0){
  //     UART.transmitByte(test_byte);
  //   }
  // }


//FOR TESTING SENDING BUFFER 
  const uint16_t test_data[] = {'T', 'E', 'S', 'T', '\0'};

  while(true){
    uint16_t length = sizeof(test_data)/sizeof(test_data[0]);
    UART.transmitBuffer(test_data, length);
    wait_ms(1000);
  }

//LEAVING THIS INCASE ITS USEFUL LATER
  // while (true)
  // {

  //   readVal = UART.readByte();
  //   status = UART.rxStatus();

  //   if (status == UARTRxStatusCode::IDLE)
  //   {
  //     // skip if there's no data
  //     continue;
  //   }
  //   if (readVal == 0)
  //   {
  //     // skip if there's no data
  //     continue;
  //   }

  //   buffer[index] = readVal;
  //   xpd_putc(readVal);

  //   if (buffer[index] == 0x0d)
  //   {

  //     int j = 0;
  //     while (buffer[j] != 0x0d)
  //     {
  //       xpd_putc(buffer[j]);
  //       j++;
  //     }

  //     xpd_putc(0x0a);

  //     for (int i = 0; i < sizeof(buffer); i++)
  //     {
  //       buffer[i] = '\0';
  //     }
  //     xpd_puts("COMMAND RECIEVED");
  //     xpd_putc(0x0a);
  //     index = 0;
  //     continue;
  //   }
  //   index++;
  // }
}
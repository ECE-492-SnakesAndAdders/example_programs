#define BOARD_C3
#include "time_funcs.h"
// #include "UART.h"

#include <Structs.h>
#include <Thread.h>
#include <XPD.h>
#include "GPIO_nio.h"
#include "UART_GPIO_NIO.h"

// GlobalPin TX_pin = {GPIO_C, io_PC0, 0x01, Polar_ActiveHigh};
// GlobalPin RX_pin = {GPIO_C, io_PC1, 0x02, Polar_ActiveHigh};

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

  // UART.initTX(&TX,1152, 8, 0, 0, 1); 

  // uint16_t buffer[30];
  // uint16_t readVal = 0;
  // uint16_t index = 0;

  // UARTRxStatusCode status = UARTRxStatusCode::IDLE;

  // uint16_t data[30] = {'C', 'A', 'U', 'S', 'A', 'L', 'I', 'T', 'Y'};

  // UART.init_X(GlobalPin *tx, uint16_t BAUD, uint16_t DATA, uint16_t PARITY, uint16_t PTYPE, uint16_t STOP)

  // UART UART;
  // UART.initTX(&TX_pin, 96, 8, 0, 0, 1);

  // UART.initRX(&RX_pin, 96, 8, 0, 0, 1);

  // UART.transmitBuffer(buffer);
  // wait_ms(1);
  // uint16_t count = 0;
  // while (data[count] != '\0')
  // {
  //   UART.transmitByte(data[count]);
  //   count++;
  // }
  // UART.transmitByte(0x0d);
  // UART.transmitByte(0x0a);

  //TX bin 101 -- "A" measure
  // while (true) //Transmit a 0xAA with in bin is 1010 1010
  // {
  //   UART.transmitByte(0x44);
  //   wait_ms(10);
  // }

  uint16_t test_data[6] = {'D', 'E', 'V', 'I', 'N'};

  // while(true){
  //   for (int i = 0; i < (sizeof(test_data) / sizeof(test_data[0])); i++){
  //     UART.transmitByte(test_data[i]);
  //   }
  //   wait_ms(1000);
  // }



  //SENDING WORKS SOMETIMES ! - But mostly picking up garbage - again this is a timing issue
  //Seems like its working now with a check that te byte being read is not empty 
  uint16_t test_byte = 0;
   while(true){
    test_byte = UART.readByte();
    if(test_byte != 0){
      UART.transmitByte(test_byte);
    }
    // wait_ms(1000);
  }






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
#ifndef _UART_GPIO_NIO_H_
#define _UART_GPIO_NIO_H_

#include <SystemClock.h>
#include <Structs.h>
#include <SFU.h>
#include <XPD.h>
#include "GPIO_nio.h"

enum UARTRxStatusCode
{
  BUSY,
  IDLE,
  DONE,
  PARITY_ERROR,
  STOPBIT_ERROR
};

enum UARTRxPinState
{
  LOW = 0,
  HIGH = 1
};

class UartTX
{
public:
  void init(GPIO *tx, uint16_t BAUD, uint16_t DATA, uint16_t PARITY, uint16_t PTYPE, uint16_t STOP)
  {
    _tx = *tx;
    baudrate = BAUD;
    dataBits = DATA;
    parityBits = PARITY;
    stopBits = STOP;
    pType = PTYPE;

    // initialize pin
    // globalPin_set_dir(GlobalPin_dir::PinDir_Output, _tx);
    // globalPin_write(PinLogicState::ON, _tx);

    _tx.setDirection(OUT);
    _tx.set();
    // init TX clock
    calculateClock(baudrate);
    makeTX = 1;
  }
  void transmitByte(uint16_t data)
  {
    if (makeTX == 0)
      return;
    startTransmit();
    transmitData(data);
    transmitParity(data);
    stopTransmit();
  }

private:
  void calculateClock(uint16_t baudrate)
  {
    // only works for 98.304MHz sys freq
    switch (baudrate)
    {
    case 96:
      // _sysTicksPerClock = 10240; // calculated by hand: ticks = T * sysfreq (1/9600 * 9830400)
      //  _sysTicksPerClock = 9568; // calculated by hand: ticks = T * sysfreq (1/9600 * 9830400)
      //Adjusted to 10240 as per Nic advice to account for delay in using the GPIOs for read and write
      //Most recent attempt: 
      _sysTicksPerClock = 8768;  //Still getting garbage 
      
      //_sysTicksPerClock = 5120;
      break;
    case 1152:
      _sysTicksPerClock = 400; // calculated by hand, adjusted manually
      break;
    default:
      // default to 9600 baud
      _sysTicksPerClock = 10240;
      break;
    }
  }

  void startTransmit()
  {
    // globalPin_write(PinLogicState::OFF, _tx);
    _tx.reset();
    waitClockCycle();
  }

  void transmitData(uint16_t data)
  {
    for (uint16_t i = 0; i < dataBits; i++)
    {
      // this sends lsB first. We can reverse it with a _ror call instead -> _ror(1, (uint16)DATABITS - 1 - i)
      if (_rol(1, i) & data)
      {
        // globalPin_write(PinLogicState::ON, _tx);
        _tx.set();
      }
      else
      {
        // globalPin_write(PinLogicState::OFF, _tx);
        _tx.reset();
      }
      waitClockCycle();
    }
  }

  void transmitParity(uint16_t data)
  {
    if (parityBits == 0)
      return;

    if (parityBits == 1)
    {

      if (pType == 1)
      {
        if (Even_calculateParity(data) > 0)
        {
          // globalPin_write(PinLogicState::ON, _tx);
          _tx.set();
        }
        else
        {
          // globalPin_write(PinLogicState::OFF, _tx);
          _tx.reset();
        }
        waitClockCycle();
      }

      if (pType == 0)
      {
        if (ODD_calculateParity(data) > 0)
        {
          // globalPin_write(PinLogicState::ON, _tx);
          _tx.set();
        }
        else
        {
          // globalPin_write(PinLogicState::OFF, _tx);
          _tx.reset();
        }
        waitClockCycle();
      }
    }
  }

  void stopTransmit()
  {
    // globalPin_write(PinLogicState::ON, _tx);
    _tx.set();
    waitClockCycle();
    if (stopBits == 2)
      waitClockCycle();
  }
  uint16_t Even_calculateParity(uint16_t data)
  {
    if (sfu_pop_count(data) % 2 == 0)
      return 0;
    return 1;
  }
  uint16_t ODD_calculateParity(uint16_t data)
  {
    if (sfu_pop_count(data) % 2 == 0)
      return 1;
    return 0;
  }

  // const uint16_t sys_ticks_per_clock = 10240; //Original setting
  const uint16_t sys_ticks_per_clock = 4470; //104us  (which is period of 1 bit for 9600baud)
  void waitClockCycle()
  {
    // sys_clock_wait(_sysTicksPerClock);
    internal_busy_wait(sys_ticks_per_clock);
  }

  void waitHalfClockCycle()
  {
    sys_clock_wait(_sysTicksPerClock / 2);
  }

  void waitQuarterClockCycle()
  {
    sys_clock_wait(_sysTicksPerClock / 4);
  }
  uint16_t baudrate = 9600;
  uint16_t dataBits = 8;
  uint16_t parityBits = 0;
  uint16_t stopBits = 2;
  uint16_t pType = 1;

  GPIO _tx;
  uint16_t _sysTicksPerClock;
  uint16_t makeTX = 0;
};

class UartRX
{
public:
  void init(GPIO *rx, uint16_t BAUD, uint16_t DATA, uint16_t PARITY, uint16_t PTYPE, uint16_t STOP)
  {
    _rx = *rx;
    baudrate = BAUD;
    dataBits = DATA;
    parityBits = PARITY;
    stopBits = STOP;
    pType = PTYPE;

    // initialize pin
    // globalPin_set_dir(GlobalPin_dir::PinDir_Input, _rx);
    _rx.setDirection(IN);
    // init RX clock
    calculateClock(baudrate);
  }
  uint16_t readByte()
  {
    reset();
    monitorForStart();
    if (_status != BUSY)
      return 0;
    readData();
    readParityBit();
    readStop();
    checkParity();
    return _read;
  }

  UARTRxStatusCode rxStatus()
  {
    return _status;
  }

private:
  void monitorForStart()
  {
    _newstate = (UARTRxPinState)_rx.readPin();
    if (_oldstate == HIGH && _newstate == LOW)
    {
      _status = BUSY;
      waitClockCycle();
      waitHalfClockCycle(); // so we read the bit half way into the bit frame.
      // waitQuarterClockCycle();
    }
    _oldstate = _newstate;
  }

  void readData()
  {
    for (uint16_t i = 0; i < dataBits; i++)
    {
      _read |= (_rx.readPin() << i);
      waitClockCycle();
    }
  }

  void readParityBit()
  {
    if (parityBits == 1)
    {
      //_parityBit = globalPin_read_raw(_rx);
      _parityBit = _rx.readPin();
      waitClockCycle();
    }
    if (parityBits == 0)
    {
      return;
    }
  }

  void readStop()
  {
    // uint16_t stopBit1 = globalPin_read_raw(_rx);
    uint16_t stopBit1 = _rx.readPin();
    uint16_t stopBit2 = 1;
    if (stopBits > 1)
    {
      waitClockCycle();
      // stopBit2 = globalPin_read_raw(_rx);
      stopBit2 = _rx.readPin();
    }
    if (stopBit1 && stopBit2)
    {
      return;
    }
    else
      _status = STOPBIT_ERROR;
  }

  void checkParity()
  {
    if (parityBits == 0)
    {
      _status = DONE;
      return;
    }
    // Works for EVEN and ODD Parity
    if (parityBits == 1)
    {

      if (parityBits == 1)
      {
        if ((sfu_pop_count(_read) % 2) == _parityBit)
        {
          _status = DONE;
        }
        else
          _status = PARITY_ERROR;
      }
      if (pType == 0)
      {
        if ((sfu_pop_count(_read) % 2) == _parityBit)
        {
          _status = PARITY_ERROR;
        }
        else
          _status = DONE;
      }
    }
  }
  // const uint16_t sys_ticks_per_clock = 10240; //Original setting
  const uint16_t sys_ticks_per_clock = 4470; //104us  (which is period of 1 bit for 9600baud)
  void waitClockCycle()
  {
    // sys_clock_wait(_sysTicksPerClock);
    internal_busy_wait(sys_ticks_per_clock);
  }

  void waitHalfClockCycle()
  {
    // sys_clock_wait(_sysTicksPerClock / 2);
    internal_busy_wait(sys_ticks_per_clock / 2);
  }

  void waitQuarterClockCycle()
  {
    // sys_clock_wait(_sysTicksPerClock / 4);
    internal_busy_wait(sys_ticks_per_clock / 4);
  }

private:
  void calculateClock(uint16_t baudrate)
  {
    // only works for 98.304MHz sys freq
    switch (baudrate)
    {
    case 96:
       _sysTicksPerClock = 9568; // calculated by hand: ticks = T * sysfreq (1/9600 * 9830400)
      //Adjusted to account for the inherent delay in using the GPIOs for read and write
      //_sysTicksPerClock = 5120;
      break;
    case 1152:
      _sysTicksPerClock = 400; // calculated by hand, adjusted manually
      break;
    default:
      // default to 9600 baud
      _sysTicksPerClock = 10240;
      break;
    }
  }

  void reset()
  {
    _read = 0;
    _parityBit = 0;
    _status = IDLE;
  }
  uint16_t baudrate = 96;
  uint16_t dataBits = 8;
  uint16_t parityBits = 0;
  uint16_t stopBits = 2;
  uint16_t pType = 1;
  GPIO _rx;
  uint16_t _sysTicksPerClock;
  UARTRxStatusCode _status = IDLE;
  UARTRxPinState _oldstate = LOW; // _oldstate and _new state serve to help detect a START condition
  UARTRxPinState _newstate = LOW;
  uint16_t _read = 0;
  uint16_t _parityBit = 0;
};

class UART
{
public:
  void initTX(GPIO *tx, uint16_t BAUD, uint16_t DATA, uint16_t PARITY, uint16_t PTYPE, uint16_t STOP)
  {
    TX.init(tx, BAUD, DATA, PARITY, PTYPE, STOP);
  }
  void initRX(GPIO *rx, uint16_t BAUD, uint16_t DATA, uint16_t PARITY, uint16_t PTYPE, uint16_t STOP)
  {
    RX.init(rx, BAUD, DATA, PARITY, PTYPE, STOP);
  }

  void transmitByte(uint16_t data)
  {
    TX.transmitByte(data);
  }

  UARTRxStatusCode rxStatus()
  {
    return RX.rxStatus();
  }

  uint16_t readByte()
  {
    _read = RX.readByte();
    return _read;
  }

private:
  UartTX TX;
  UartRX RX;

  uint16_t _read = 0;
};

#endif // _UART_GPIO_NIO_H_

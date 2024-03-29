#ifndef _UART_H_
#define _UART_H_

#include <SystemClock.h>
#include <Structs.h>
#include <SFU.h>
#include <XPD.h>

class UartTX
{
public:
  void init(GlobalPin *tx, uint16_t BAUD, uint16_t DATA, uint16_t PARITY, uint16_t PTYPE, uint16_t STOP)
  {
    _tx = tx;
    baudrate = BAUD;
    dataBits = DATA;
    parityBits = PARITY;
    stopBits = STOP;
    pType = PTYPE;

    // initialize pin
    globalPin_set_dir(GlobalPin_dir::PinDir_Output, _tx);
    globalPin_write(PinLogicState::ON, _tx);
    // init TX clock
    calculateClock(baudrate);
  }
  void transmitByte(uint16_t data)
  {
    if (_tx == nullptr)
      return;
    startTransmit();
    transmitData(data);
    transmitParity(data);
    stopTransmit();
  }

//TODO - Calculate ticks by clock for using the C3 system clock (~24MHz)
private:
  void calculateClock(uint16_t baudrate)
  {
    // only works for 98.304MHz sys freq
    switch (baudrate)
    {
    case 96:
    //Need to account for GPIO rx/tx
      _sysTicksPerClock = 10240; // calculated by hand: ticks = T * sysfreq (1/9600 * 9830400)
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
    globalPin_write(PinLogicState::OFF, _tx);
    waitClockCycle();
  }

  void transmitData(uint16_t data)
  {
    for (uint16_t i = 0; i < dataBits; i++)
    {
      // this sends lsB first. We can reverse it with a _ror call instead -> _ror(1, (uint16)DATABITS - 1 - i)
      if (_rol(1, i) & data)
      {
        globalPin_write(PinLogicState::ON, _tx);
      }
      else
      {
        globalPin_write(PinLogicState::OFF, _tx);
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
          globalPin_write(PinLogicState::ON, _tx);
        }
        else
        {
          globalPin_write(PinLogicState::OFF, _tx);
        }
        waitClockCycle();
      }

      if (pType == 0)
      {
        if (ODD_calculateParity(data) > 0)
        {
          globalPin_write(PinLogicState::ON, _tx);
        }
        else
        {
          globalPin_write(PinLogicState::OFF, _tx);
        }
        waitClockCycle();
      }
    }
  }

  void stopTransmit()
  {
    globalPin_write(PinLogicState::ON, _tx);
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

  void waitClockCycle()
  {
    sys_clock_wait(_sysTicksPerClock);
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

  GlobalPin *_tx = nullptr;
  uint16_t _sysTicksPerClock;
};

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

class UartRX
{
public:
  void init(GlobalPin *rx, uint16_t BAUD, uint16_t DATA, uint16_t PARITY, uint16_t PTYPE, uint16_t STOP)
  {
    _rx = rx;
    baudrate = BAUD;
    dataBits = DATA;
    parityBits = PARITY;
    stopBits = STOP;
    pType = PTYPE;

    // initialize pin
    globalPin_set_dir(GlobalPin_dir::PinDir_Input, _rx);
    // init RX clock
    calculateClock(baudrate);
  }
  uint16_t readByte()
  {
    reset();
    monitorForStart();
    if (_status != UARTRxStatusCode::BUSY)
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
    _newstate = (UARTRxPinState)globalPin_read_raw(_rx);
    if (_oldstate == UARTRxPinState::HIGH && _newstate == UARTRxPinState::LOW)
    {
      _status = UARTRxStatusCode::BUSY;
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
      _read = _read | (globalPin_read_raw(_rx) << i);
      waitClockCycle();
    }
  }

  void readParityBit()
  {
    if (parityBits == 1)
    {
      _parityBit = globalPin_read_raw(_rx);
      waitClockCycle();
    }
    if (parityBits == 0)
    {
      return;
    }
  }

  void readStop()
  {
    uint16_t stopBit1 = globalPin_read_raw(_rx);
    uint16_t stopBit2 = 1;
    if (stopBits > 1)
    {
      waitClockCycle();
      stopBit2 = globalPin_read_raw(_rx);
    }
    if (stopBit1 && stopBit2)
    {
      return;
    }
    else
      _status = UARTRxStatusCode::STOPBIT_ERROR;
  }

  void checkParity()
  {
    if (parityBits == 0)
    {
      _status = UARTRxStatusCode::DONE;
      return;
    }
    // Works for EVEN and ODD Parity
    if (parityBits == 1)
    {

      if (parityBits == 1)
      {
        if ((sfu_pop_count(_read) % 2) == _parityBit)
        {
          _status = UARTRxStatusCode::DONE;
        }
        else
          _status = UARTRxStatusCode::PARITY_ERROR;
      }
      if (pType == 0)
      {
        if ((sfu_pop_count(_read) % 2) == _parityBit)
        {
          _status = UARTRxStatusCode::PARITY_ERROR;
        }
        else
          _status = UARTRxStatusCode::DONE;
      }
    }
  }
  void waitClockCycle()
  {
    sys_clock_wait(_sysTicksPerClock);
  }

  void waitHalfClockCycle()
  {
    sys_clock_wait(_sysTicksPerClock / 2);
  }

  void waitQuarterClockCycle()
  {
    sys_clock_wait(_sysTicksPerClock / 4);
  }


//TODO - Calculate ticks by clock for using the C3 system clock (~24MHz)
private:
  void calculateClock(uint16_t baudrate)
  {
    // only works for 98.304MHz sys freq
    switch (baudrate)
    {
    case 96:
      // _sysTicksPerClock = 10240; // calculated by hand: ticks = T * sysfreq (1/9600 * 9830400)
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
    _status = UARTRxStatusCode::IDLE;
  }
  uint16_t baudrate = 9600;
  uint16_t dataBits = 8;
  uint16_t parityBits = 0;
  uint16_t stopBits = 2;
  uint16_t pType = 1;
  GlobalPin *_rx = nullptr;
  uint16_t _sysTicksPerClock;
  UARTRxStatusCode _status = UARTRxStatusCode::IDLE;
  UARTRxPinState _oldstate = UARTRxPinState::LOW; // _oldstate and _new state serve to help detect a START condition
  UARTRxPinState _newstate = UARTRxPinState::LOW;
  uint16_t _read = 0;
  uint16_t _parityBit = 0;
};

class UART
{
public:
  void initTX(GlobalPin *tx, uint16_t BAUD, uint16_t DATA, uint16_t PARITY, uint16_t PTYPE, uint16_t STOP)
  {
    TX.init(tx, BAUD, DATA, PARITY, PTYPE, STOP);
  }
  void initRX(GlobalPin *rx, uint16_t BAUD, uint16_t DATA, uint16_t PARITY, uint16_t PTYPE, uint16_t STOP)
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

#endif // _UART_H_

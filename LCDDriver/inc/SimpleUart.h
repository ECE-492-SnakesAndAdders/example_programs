#ifndef _SIMPLE_UART_H_
#define _SIMPLE_UART_H_

#include <SystemClock.h>
#include <Structs.h>
#include <SFU.h>

namespace UARTProtocol {

  enum class BaudRate : uint16_t {
    _9600 = 96,
    _115200 = 1152
  };

  enum class DataBits : uint16_t {
    _5 = 5,
    _6 = 6,
    _7 = 7,
    _8 = 8,
    _9 = 9
  };

  enum class ParityBits : uint16_t {
    // only works for even parity
    _0 = 0,
    _1 = 1
  };
  enum class TParity : uint16_t{
  ODD = 0,
  EVEN = 1
  };

  enum class StopBits : uint16_t {
    _1 = 1,
    _2 = 2
  };

  uint16_t Even_calculateParity(uint16_t data) {
    if (sfu_pop_count(data) % 2 == 0) return 0;
    return 1;
  }
  uint16_t ODD_calculateParity(uint16_t data) {
    if (sfu_pop_count(data) % 2 == 0) return 1;
    return 0;
  }

}


template<UARTProtocol::BaudRate BAUD, UARTProtocol::DataBits DATABITS, UARTProtocol::ParityBits PARITYBITS, UARTProtocol :: TParity ParityType ,UARTProtocol::StopBits STOPBITS>
class UartTX
{
  public:
  void init(GlobalPin*  tx) 
  {
    _tx = tx;
    // initialize pin
    globalPin_set_dir(GlobalPin_dir::PinDir_Output, _tx);
    globalPin_write(PinLogicState::ON, _tx);
    // init TX clock
    calculateClock();
  }

  void transmit(uint16_t data)
  {
    if (_tx == nullptr) return;
    startTransmit();
    transmitData(data);
    transmitParity(data);
    stopTransmit();
  }

  private:
  void startTransmit()
  {
    globalPin_write(PinLogicState::OFF, _tx);
    waitClockCycle();
  }

  void transmitData(uint16_t data)
  {
    for (uint16_t i = 0; i < ((uint16_t)DATABITS); i++) {
      // this sends lsB first. We can reverse it with a _ror call instead -> _ror(1, (uint16)DATABITS - 1 - i)
      if (_rol(1,i) & data) {
        globalPin_write(PinLogicState::ON, _tx);
      } else {
        globalPin_write(PinLogicState::OFF, _tx);
      }
      waitClockCycle();
    }
  }

  void transmitParity(uint16_t data)
  {
    if ((uint16_t)PARITYBITS == 0) return;

    if ((uint16_t)PARITYBITS == 1) {

    if ((uint16_t) ParityType == 1) {
    if (UARTProtocol::Even_calculateParity(data) > 0) {
      globalPin_write(PinLogicState::ON, _tx);
    } else {
      globalPin_write(PinLogicState::OFF, _tx);
    }
    waitClockCycle();
    }

    if ((uint16_t)ParityType == 0) {
      if (UARTProtocol::ODD_calculateParity(data) > 0) {
      globalPin_write(PinLogicState::ON, _tx);
    } else {
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
    if ((uint16_t)STOPBITS == 2) waitClockCycle();
  }

  void waitClockCycle()
  {
    sys_clock_wait(_sysTicksPerClock);
  }

  void calculateClock()
  {
    // only works for 98.304MHz sys freq
    switch ((uint16_t)BAUD)
    {
    case 96:
      _sysTicksPerClock = 10240; // calculated by hand: ticks = T * sysfreq (1/9600 * 9830400)
      break;
    case 1152:
      _sysTicksPerClock = 400; // calculated by hand, adjusted manually
      break;
    default:
      // default to 9600 baud
      _sysTicksPerClock = 10240;
      break;
    }
    // xpd_puts("BAUD: ");
    // xpd_echo_int((uint16_t)BAUD, XPD_Flag_UnsignedDecimal);
    // xpd_puts("\nsysTicksPerClock: ");
    // xpd_echo_int(_sysTicksPerClock, XPD_Flag_UnsignedDecimal);
  }

  GlobalPin* _tx = nullptr;
  uint16_t _sysTicksPerClock;

};

enum UARTRxStatusCode {
  BUSY,
  IDLE,
  DONE,
  PARITY_ERROR,
  STOPBIT_ERROR
};

enum UARTRxPinState {
  LOW = 0,
  HIGH = 1 
};

template<UARTProtocol::BaudRate BAUD, UARTProtocol::DataBits DATABITS, UARTProtocol::ParityBits PARITYBITS, UARTProtocol :: TParity ParityType, UARTProtocol::StopBits STOPBITS>
class UartRX
{
  // This does not work great at 115200 BAUD
  public:
  void init(GlobalPin* rx) 
  {
    _rx = rx;
    // initialize pin
    globalPin_set_dir(GlobalPin_dir::PinDir_Input, _rx);
    // init RX clock
    calculateClock();
  }

  uint16_t read()
  {
    reset();
    monitorForStart();
    if (_status != UARTRxStatusCode::BUSY) return 0;
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
    if (_oldstate == UARTRxPinState::HIGH && _newstate == UARTRxPinState::LOW) {
      _status = UARTRxStatusCode::BUSY;
      waitClockCycle();
      waitHalfClockCycle(); // so we read the bit half way into the bit frame.
      // waitQuarterClockCycle();
    }
    _oldstate = _newstate;
  }

  void readData()
  {
    for (uint16_t i = 0; i < ((uint16_t)DATABITS); i++) {
      _read = _read | (globalPin_read_raw(_rx) << i);
      waitClockCycle();
    }
  }

  void readParityBit()
  {
    if ((uint16_t)PARITYBITS == 1){
    _parityBit = globalPin_read_raw(_rx);
    waitClockCycle();
    }
    if ((uint16_t)PARITYBITS == 0) {
      return;
    }

  }
  
  void readStop()
  {
    uint16_t stopBit1 = globalPin_read_raw(_rx);
    uint16_t stopBit2 = 1;
    if ((uint16_t)STOPBITS > 1) {
      waitClockCycle();
      stopBit2 = globalPin_read_raw(_rx);
    }
    if (stopBit1 && stopBit2){
      return;
    } else _status = UARTRxStatusCode::STOPBIT_ERROR;
  }

  void checkParity()
  {
    if ((uint16_t)PARITYBITS == 0) {
      _status = UARTRxStatusCode::DONE;
      return;
    }
    // Works for EVEN and ODD Parity 
    if ((uint16_t)PARITYBITS == 1) {

    if ((uint16_t)ParityType == 1) {
    if ((sfu_pop_count(_read) % 2) == _parityBit) {
      _status = UARTRxStatusCode::DONE;
    } else _status = UARTRxStatusCode::PARITY_ERROR;
    }
    if ((uint16_t)ParityType == 0) {
    if ((sfu_pop_count(_read) % 2) == _parityBit) {
      _status = UARTRxStatusCode::PARITY_ERROR;
    } else _status = UARTRxStatusCode::DONE;
    }
    }

  }

  void calculateClock()
  {
    // only works for 98.304MHz sys freq
    switch ((uint16_t)BAUD)
    {
    case 96:
      _sysTicksPerClock = 10240; // calculated by hand: ticks = T * sysfreq (1/9600 * 9830400)
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

  void reset()
  {
    _read = 0;
    _parityBit = 0;
    _status = UARTRxStatusCode::IDLE;
  }

  GlobalPin* _rx = nullptr;
  uint16_t _sysTicksPerClock;
  UARTRxStatusCode _status = UARTRxStatusCode::IDLE;
  UARTRxPinState _oldstate = UARTRxPinState::LOW; // _oldstate and _new state serve to help detect a START condition
  UARTRxPinState _newstate = UARTRxPinState::LOW;
  uint16_t _read = 0;
  uint16_t _parityBit = 0;
};

template<typename TXType, typename RXType>
class UART
{
  public:
  void init(GlobalPin* tx, GlobalPin* rx)
  {
    _tx.init(tx);
    _rx.init(rx);
  }

  void transmit(uint16_t data)
  {
    _tx.transmit(data);
  }

  uint16_t read()
  {
    _read = _rx.read();
    // heres where function calls go to check parity etc.
    return _read;
  }

  UARTRxStatusCode rxStatus()
  {
    return _rx.rxStatus();
  }

  private:
  TXType _tx;
  RXType _rx;
  uint16_t _read = 0;
};




#endif // _SIMPLE_UART_H_


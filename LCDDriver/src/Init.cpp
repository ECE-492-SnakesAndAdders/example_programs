#include <SystemClock.h>
#include <IOConfig.h>


// Board initialization routine
// Any code written here will be called before main() is entered.
void __attribute__((constructor)) Initialize()
{
  // Configure system clock for C3 dev board - uses 24MHz clock
  sys_clock_init(crys_24_576_MHz, _49_152_MHz);

  // Configure a pull-up on pin PG0
  // This pull-up prevents an XPD write from stalling execution if
  // there is no XPD attached.
  io_set_config(0b1110011, io_PG0);
}

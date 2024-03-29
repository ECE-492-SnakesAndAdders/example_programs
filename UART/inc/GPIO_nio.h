#ifndef _GPIO_NIO_H_
#define _GPIO_NIO_H_

#include <GPIO.h> // the older one

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Enumeratation section:
// These are all the available ports.
enum PORT
{
    PORT_A = 0,
    PORT_B = 1,
    PORT_C = 2,
    PORT_D = 3,
    PORT_E = 4,
    PORT_F = 5,
    PORT_G = 6,
    PORT_H = 7,
    PORT_I = 8,
    PORT_J = 9,
    NO_PORT = 10,
};

// Input or Output
enum Direction
{
    IN = 0,
    OUT = 1,
    NO_DIR = 2,
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This is the main class for GPIO. You must refer to it if you wish to call any of the functions that it contains
// example code:
//
// GPIO A0; -> A0 is the defined member of the class GPIO. You can change it from A0 to other names
// A0.init(PORT_A,3,IN); -> The name you chose earlier followed by period (.) and then the function and its arguments.
// A0.set(); -> Same as above but this function doesnt need any arguments.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This is the main class for GPIO. You must refer to it if you wish to call any of the functions that it contains
// example code:
//
// GPIO A0; -> A0 is the defined member of the class GPIO. You can change it from A0 to other names
// A0.init(PORT_A,3,IN); -> The name you chose earlier followed by period (.) and then the function and its arguments.
// A0.set(); -> Same as above but this function doesnt need any arguments.

class GPIO
{
public:
    // list of all the available functions:
    void init(PORT port, uint16_t pin, Direction pin_dir); // initializes and configures the pin
    void set();                                            // sets the pin high
    void reset();                                          // sets the pin low
    void toggle();                                         // flips the pin value
    void setDirection(Direction pin_dir);                  // sets the direction of the pin
    uint16_t readPin();                                    // Returns 1 if pin is high, 0 if pin is low
    uint16_t readPort();                                   // Returns 8 bit value of the entire port

    PORT getPort();           // Returns the port
    uint16_t getPin();        // Returns the pin
    Direction getDirection(); // Returns the pin direction

private:
    // these variables are internal to the class only. Must refer to the class if you wish to use it.
    GPIO_port_name UnwrapPortname(PORT port); // Here to properly comunicate with the older GPIO driver
    PORT _port;                               // stores the port name for use in subsequent function calls
    uint16_t _pin;                            // stores pin number for use in subsequent function calls
    Direction _pin_dir;                       // stores pin direction for use in subsequent function calls
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// this function maintains backwards compatibility with old header file
GPIO_port_name GPIO::UnwrapPortname(PORT port)
{
    switch (port)
    {
    case PORT_A:
        return GPIO_A;
    case PORT_B:
        return GPIO_B;
    case PORT_C:
        return GPIO_C;
    case PORT_D:
        return GPIO_D;
    case PORT_E:
        return GPIO_E;
    case PORT_F:
        return GPIO_F;
    case PORT_G:
        return GPIO_G;
    case PORT_H:
        return GPIO_H;
    case PORT_I:
        return GPIO_I;
    case PORT_J:
        return GPIO_J;
    }
}

// this function configures the given port and pin combination to the specified direction. (input or output)
void GPIO::init(PORT port, uint16_t pin, Direction pin_dir)
{
    switch (port) // Testing to see if the pin and port entered exist
    {
    case PORT_A:
        if (!(pin >= 0 && pin <= 7))
        {
            xpd_puts("PA Invalid pin and port combination\n");
            return;
        }
        break;

    case PORT_B:
        if (!(pin >= 0 && pin <= 1))
        {
            xpd_puts("PB Invalid pin and port combination\n");
            return;
        }
        break;

    case PORT_C:
        if (!(pin >= 0 && pin <= 7))
        {
            xpd_puts("PC Invalid pin and port combination\n");
            return;
        }
        break;

    case PORT_D:
        if (!(pin >= 0 && pin <= 7))
        {
            xpd_puts("Invalid pin and port combination\n");
            return;
        }
        break;

    case PORT_E:
        if (!(pin >= 0 && pin <= 7))
        {
            xpd_puts("Invalid pin and port combination\n");
            return;
        }
        break;

    case PORT_F:
        if (!(pin >= 0 && pin <= 1))
        {
            xpd_puts("Invalid pin and port combination\n");
            return;
        }
        break;

    case PORT_G:
        if (!(pin >= 0 && pin <= 2))
        {
            xpd_puts("Invalid pin and port combination\n");
            return;
        }
        break;

    case PORT_H:
        if (!(pin >= 0 && pin <= 2))
        {
            xpd_puts("Invalid pin and port combination\n");
            return;
        }
        break;

    case PORT_I:
        if (!(pin >= 0 && pin <= 2))
        {
            xpd_puts("Invalid pin and port combination\n");
            return;
        }
        break;

    case PORT_J:
        if (!(pin >= 0 && pin <= 3))
        {
            xpd_puts("Invalid pin and port combination\n");
            return;
        }
        break;
    default:
        break;
    }

    // if the entered arguments are valid, store them in private variables for future functions
    _port = port;
    _pin = pin;
    _pin_dir = pin_dir;

    uint16_t state = gpio_get_config(UnwrapPortname(_port)); // read the current state of the port

    if (_pin_dir == OUT)
    {
        state |= (1 << (_pin + 8));                    // change the pin bit in the port to out
        gpio_set_config(state, UnwrapPortname(_port)); // push the new direction of the pin
    }
    else
    {
        state &= ~(1 << (_pin + 8));                   // change the pin bit in the port to in
        gpio_set_config(state, UnwrapPortname(_port)); // push the new direction of the pin
    }
}

// this function sets the configured pin
void GPIO::set()
{
    uint16_t state = gpio_get_config(UnwrapPortname(_port)); // read current value of all pins on the port
    state |= (1 << _pin);                                    // create a bitmask to change value of the pin
    gpio_write(state, UnwrapPortname(_port));                // push the new value
}

// this function resets the configured pin
void GPIO::reset()
{
    uint16_t state = gpio_get_config(UnwrapPortname(_port)); // read current value of all pins on the port
    state &= ~(1 << _pin);                                   // create a bitmask to change value of the pin
    gpio_write(state, UnwrapPortname(_port));                // push the new value
}

// this function resets a set pit and vice versa
void GPIO::toggle()
{
    uint16_t state = gpio_get_config(UnwrapPortname(_port)); // read current value of all pins on the port
    state ^= (1 << _pin);                                    // creates a bitmask to flip the value of the pin
    gpio_write(state, UnwrapPortname(_port));                // push the new value
}

// this function changes an input pin to output and vice versa
//  Hasnt been tested properly yet
void GPIO::setDirection(Direction pin_dir)
{
    _pin_dir = pin_dir;                                      // The new direction the user wants
    uint16_t state = gpio_get_config(UnwrapPortname(_port)); // read the current state of the port
    if (_pin_dir == OUT)
    {
        state |= (1 << (_pin + 8));                    // change the pin bit in the port to out
        gpio_set_config(state, UnwrapPortname(_port)); // push the new direction of the pin
    }
    else
    {
        state &= ~(1 << (_pin + 8));                   // change the pin bit in the port to in
        gpio_set_config(state, UnwrapPortname(_port)); // push the new direction of the pin
    }
}

// this function reads the current state of the pin
uint16_t GPIO::readPin()
{
    uint16_t port_value = gpio_read(UnwrapPortname(_port));
    uint16_t pin_value = port_value & (1 << _pin);
    if (pin_value)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// this function reads the current state of the entire port
uint16_t GPIO::readPort()
{
    return gpio_read(UnwrapPortname(_port));
}

// this function returns the most recent port configured
PORT GPIO::getPort()
{
    return _port;
}

// this function returns the most recent pin configured
uint16_t GPIO::getPin()
{
    return _pin;
}

// this function returns the direction of the most recent pin configured
Direction GPIO::getDirection()
{
    return _pin_dir;
}

#endif //_GPIO_NIO_H
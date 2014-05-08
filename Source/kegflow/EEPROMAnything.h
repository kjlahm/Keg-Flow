#include <EEPROM.h>
#include <Arduino.h>  // for type definitions

// Writes the given array/structure to the given memory address
// @param ee The address to write to
// @param value The array/structure to be written
// @returns The number of bytes written
template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          EEPROM.write(ee++, *p++);
    return i;
}

// Reads data from memory into an array/struct
// @param ee The memory address to read from
// @param value The array/struct to read data into (output)
// @returns The number of bytes read
template <class T> int EEPROM_readAnything(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          *p++ = EEPROM.read(ee++);
    return i;
}

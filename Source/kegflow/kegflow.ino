//
// File name:    kegflow.ino
// Creator:      Ken Lahm <kjlahm@gmail.com>
//
// Created:      19 July 2012
// Last Edited:  6 May 2014
// Version:      2.0
//
//
// Description:
//    This program takes input from liquid flow sensors and a temperature
//    sensor inside a kegerator every 1 ms and processes the information.
//

// Include the necesary libraries:

#include <OneWire.h>
#include <EEPROM.h>

#include "FlowSensor.h"
#include "StateManager.h"

// Define the LCD
//LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
//#define REDLITE 3
//#define GREENLITE 5
//#define BLUELITE 6
//#define BACKLIGHT 13

// Define the input buttons
//#define PIN_LEFT_BUTTON A3
//#define PIN_MIDDLE_BUTTON A4
//#define PIN_RIGHT_BUTTON A5
#define PIN_SAVE_BUTTON A2

#define DELAY 50

int SAVE_pressCount = 0;
int SAVE_lastReading = 0;
int SAVE_state = 0;
int SAVE_lastState = 0;

// Define the memory state manager
#define EE_ADDR_CONFIG 0
StateManager stateManager(EE_ADDR_CONFIG);

// Define the temperature sensor
#define PIN_TEMP_SENSOR A0
OneWire ds(PIN_TEMP_SENSOR);

// Define the left tap flow meter
#define PIN_LEFT_TAP_FLOW_METER 4
FlowSensor leftTapSensor;

// Define the right tap flow meter
#define PIN_RIGHT_TAP_FLOW_METER 2
FlowSensor rightTapSensor;

// Declare variables for serial communication
uint8_t serialIndex = 0;
#define COMMAND_BUFFER_SIZE 25
char commandBuffer[COMMAND_BUFFER_SIZE];


/*
 * Interrupt is called once a millisecond, looks for any pulses from the sensor!
 */
SIGNAL(TIMER0_COMPA_vect) {
  leftTapSensor.checkSensor();
  rightTapSensor.checkSensor();
}

/*
 * Enables or disables the interrupts.
 */
void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
  }
}

/*
 *
 */
void setup() {
  Serial.begin(9600);
  Serial.println("Begin");
  
  // Initialize the input buttons
  pinMode(PIN_SAVE_BUTTON, INPUT);
  digitalWrite(PIN_SAVE_BUTTON, HIGH);
  
  // Initialize the left tap flow meter
  leftTapSensor.init(PIN_LEFT_TAP_FLOW_METER);
  
  // Initialize the right tap flow meter
  rightTapSensor.init(PIN_RIGHT_TAP_FLOW_METER);
  
  // Attempt to read any existing saved data
  readStateInformation();
  
  // Enable the interrupts
  useInterrupt(true);
}

/*
 *
 */
void loop() {
  // Check for any user input
  check_buttons();
  
  // Check for serial input
  check_commands();
}


/////////////////////////////////////////////////////////////////////////////
//
// Functions for controlling the unit state.
//
/////////////////////////////////////////////////////////////////////////////

/*
 *
 */
void check_buttons() {
  int SAVE_reading = !digitalRead(PIN_SAVE_BUTTON);

  if (SAVE_lastReading == SAVE_reading) {
    SAVE_pressCount++;
  }

  if (SAVE_pressCount >= DELAY) {
    SAVE_state = 1;
  }

  if (SAVE_state == 1 && SAVE_lastState == 0) {
    saveStateInformation();
  }

  if (!SAVE_reading) {
    SAVE_pressCount = 0;
    SAVE_state = 0;
  }

  SAVE_lastState = SAVE_state;
  SAVE_lastReading = SAVE_reading;
}

/*
 * Saves pertinent program state information to non volatile memory.
 */
void saveStateInformation() {
}

/*
 * Reads program state information from non volatile memory
 */
void readStateInformation() {
}


/////////////////////////////////////////////////////////////////////////////
//
// Functions getting commands via serial.
//
/////////////////////////////////////////////////////////////////////////////

/*
 * Check the serial input line for new commands.
 */
void check_commands() {
  if (Serial.available()) {
    delay(50);
    while (Serial.available() && serialIndex < COMMAND_BUFFER_SIZE -1) {
      commandBuffer[serialIndex++] = Serial.read();
    }
    commandBuffer[serialIndex++] = '\0';
    
    if (serialIndex > 0) {
      parse_command();
      serialIndex = 0;
    }
  }
}

/*
 *
 */
void parse_command() {
  // Copy command into seperate buffer
  char command[2];
  command[0] = commandBuffer[0];
  command[1] = commandBuffer[1];
  
  // Handle save to nonvol
  if (command[0] == 'S' && command[1] == 'V') {
    saveStateInformation();
    
  // Handle reset keg command
  } else if (command[0] == 'R' && command[1] == 'K') {
    if (commandBuffer[3] == 'L') {
      leftTapSensor.setPulses(0);
    } else {
      rightTapSensor.setPulses(0);
    }
    
  // Handle set keg level (beers)
  } else if (command[0] == 'S' && command[1] == 'L') {
    int a = ((int) commandBuffer[5]) -48;
    int b = ((int) commandBuffer[6]) -48;
    int c = ((int) commandBuffer[8]) -48;
    
    float beers = (a * 10.0) + (b * 1.0) + (float)(c / 10.0);
    
    if (commandBuffer[3] == 'L') {
      leftTapSensor.setBeersLeft(beers);
    } else {
      rightTapSensor.setBeersLeft(beers);
    }
    
  // Handle set keg level (pulses)
  } else if (command[0] == 'S' && command[1] == 'P') {
    int a = ((int) commandBuffer[5]) -48;
    int b = ((int) commandBuffer[6]) -48;
    int c = ((int) commandBuffer[7]) -48;
    int d = ((int) commandBuffer[8]) -48;
    
    uint16_t pulses = (a * 1000) + (b * 100) + (c * 10) + (d * 1);
    
    if (commandBuffer[3] == 'L') {
      leftTapSensor.setPulses(pulses);
    } else {
      rightTapSensor.setPulses(pulses);
    }
    
  // Handle get keg level (beers)
  } else if (command[0] == 'G' && command[1] == 'L') {
    if (commandBuffer[3] == 'L') {
      Serial.println(leftTapSensor.getBeersLeft());
    } else {
      Serial.println(rightTapSensor.getBeersLeft());
    }
  
  // Handle get keg level (pulses)
  } else if (command[0] == 'G' && command[1] == 'P') {
    if (commandBuffer[3] == 'L') {
      Serial.println(leftTapSensor.getPulses());
    } else {
      Serial.println(rightTapSensor.getPulses());
    }
    
  // Handle command not recognized
  } else {
    Serial.print("Command not recognized: ");
    Serial.println(command);
  }
}


/////////////////////////////////////////////////////////////////////////////
//
// Functions for interacting with the temp sensor.
//
/////////////////////////////////////////////////////////////////////////////

/*
 * Returns the temperature from one DS18S20 in DEG Fahrenheit
 */
float getTemp(){
  
  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE); // Read Scratchpad

  
  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }
  
  ds.reset_search();
  
  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  
  // convert to fahrenheit
  TemperatureSum = ((9.0/5.0) * TemperatureSum) + 32.0;
  
  return TemperatureSum;
}

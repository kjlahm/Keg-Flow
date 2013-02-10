//
// File name:    kegflow.ino
// Creator:      Ken Lahm <kjlahm@gmail.com>
//
// Created:      19 July 2012
// Last Edited:  6 August 2012
// Version:      1.0
//
//
// Description:
//    This program takes input from liquid flow sensors and a temperature
//    sensor inside a kegerator every 1 ms and processes the information.
//    The processed information is then sent to an LCD for the user to view.
//

// Include the necesary libraries:

#include <LiquidCrystal.h>
#include <OneWire.h>
#include <EEPROM.h>
#include "FlowSensor.h"
#include "EEPROMAnything.h"

// Define the LCD
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
uint16_t lcdCount;
#define LCD_COUNT_MAX 1000
int brightness = 255;
#define REDLITE 3
#define GREENLITE 5
#define BLUELITE 6
#define BACKLIGHT 13

// Define the input buttons
#define PIN_LEFT_BUTTON A3
#define PIN_MIDDLE_BUTTON A4
#define PIN_RIGHT_BUTTON A5
#define PIN_SAVE_BUTTON A2

#define DELAY 50

int LEFT_pressCount = 0;
int LEFT_lastReading = 0;
int LEFT_state = 0;
int LEFT_lastState = 0;

int MIDDLE_pressCount = 0;
int MIDDLE_lastReading = 0;
int MIDDLE_state = 0;
int MIDDLE_lastState = 0;

int RIGHT_pressCount = 0;
int RIGHT_lastReading = 0;
int RIGHT_state = 0;
int RIGHT_lastState = 0;

int SAVE_pressCount = 0;
int SAVE_lastReading = 0;
int SAVE_state = 0;
int SAVE_lastState = 0;

// Define state variables
#define STATE_MAIN 0b0000
#define STATE_OFF 0b0001

#define STATE_LEFT_DETAILS 0b1000
#define STATE_LEFT_SELECT 0b1011
#define STATE_LEFT_NEW_CONFIRM 0b1001
#define STATE_LEFT_RESET_CONFIRM 0b1010

#define STATE_RIGHT_DETAILS 0b0100
#define STATE_RIGHT_SELECT 0b0111
#define STATE_RIGHT_NEW_CONFIRM 0b0101
#define STATE_RIGHT_RESET_CONFIRM 0b0110

int programState = STATE_MAIN;

#define LEFT_BUTTON 0b001
#define MIDDLE_BUTTON 0b010
#define RIGHT_BUTTON 0b100

// Define beer variables
int beerSelectIndex = 0;
int LEFT_beerCurrentIndex = 2;
int RIGHT_beerCurrentIndex = 0;

#define BEER_LIST_LENGTH 7
#define BEER_NAME_LENGTH 19
char BEER_LIST[BEER_LIST_LENGTH][BEER_NAME_LENGTH] = {
{ "Boulevard Wheat" },
{ "Brown Ale" },
{ "Empyrean" },
{ "Homebrew" },
{ "Scottish Ale" },
{ "Budweiser" },
{ "Horse Piss" }
};

// Define the temperature sensor
#define PIN_TEMP_SENSOR A0
OneWire ds(PIN_TEMP_SENSOR);

// Define the left tap flow meter
#define PIN_LEFT_TAP_FLOW_METER 4
FlowSensor leftTapSensor;

// Define the right tap flow meter
#define PIN_RIGHT_TAP_FLOW_METER 2
FlowSensor rightTapSensor;

// Define address constants
#define EE_ADDR_CONFIG 0

// Define a struct to hold program state data for saving to and reading from non volatile memory
struct config_t {
  int leftIndex;
  int rightIndex;
  uint16_t leftPulses;
  uint16_t rightPulses;
} config;

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
  
  // Initialize the LCD screen
  pinMode(BACKLIGHT, OUTPUT);
  digitalWrite(BACKLIGHT, HIGH);
  lcd.begin(20, 4);
  setBacklight(255,0,0);
  
  // Initialize the input buttons
  pinMode(PIN_LEFT_BUTTON, INPUT);
  digitalWrite(PIN_LEFT_BUTTON, HIGH);
  
  pinMode(PIN_MIDDLE_BUTTON, INPUT);
  digitalWrite(PIN_MIDDLE_BUTTON, HIGH);
  
  pinMode(PIN_RIGHT_BUTTON, INPUT);
  digitalWrite(PIN_RIGHT_BUTTON, HIGH);
  
  pinMode(PIN_SAVE_BUTTON, INPUT);
  digitalWrite(PIN_SAVE_BUTTON, HIGH);
  
  // Initialize the left tap flow meter
  leftTapSensor.init(PIN_LEFT_TAP_FLOW_METER);
  
  // Initialize the right tap flow meter
  rightTapSensor.init(PIN_RIGHT_TAP_FLOW_METER);
  
  // Attempt to read any existing saved data
  readStateInformation();
  
  // Show initial LCD
  change_LCD();
  
  // Enable the interrupts
  useInterrupt(true);
}

/*
 *
 */
void loop() {
  // Check for any user input
  check_buttons();
  
  // Update the LCD
  update_LCD();
  
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
  int LEFT_reading = !digitalRead(PIN_LEFT_BUTTON);
  int MIDDLE_reading = !digitalRead(PIN_MIDDLE_BUTTON);
  int RIGHT_reading = !digitalRead(PIN_RIGHT_BUTTON);
  int SAVE_reading = !digitalRead(PIN_SAVE_BUTTON);
  
  if (LEFT_lastReading == LEFT_reading) {
    LEFT_pressCount++;
  }
  if (MIDDLE_lastReading == MIDDLE_reading) {
    MIDDLE_pressCount++;
  }
  if (RIGHT_lastReading == RIGHT_reading) {
    RIGHT_pressCount++;
  }
  if (SAVE_lastReading == SAVE_reading) {
    SAVE_pressCount++;
  }
  
  if (LEFT_pressCount >= DELAY) {
    LEFT_state = 1;
  }
  if (MIDDLE_pressCount >= DELAY) {
    MIDDLE_state = 1;
  }
  if (RIGHT_pressCount >= DELAY) {
    RIGHT_state = 1;
  }
  if (SAVE_pressCount >= DELAY) {
    SAVE_state = 1;
  }
  
  if (LEFT_state == 1 && LEFT_lastState == 0) {
    // update state left button press
    updateState(LEFT_BUTTON);
  }
  if (MIDDLE_state == 1 && MIDDLE_lastState == 0) {
    // update state middle button press
    updateState(MIDDLE_BUTTON);
  }
  if (RIGHT_state == 1 && RIGHT_lastState == 0) {
    // update state right button press
    updateState(RIGHT_BUTTON);
  }
  if (SAVE_state == 1 && SAVE_lastState == 0) {
    saveStateInformation();
  }
  
  if (!LEFT_reading) {
    LEFT_pressCount = 0;
    LEFT_state = 0;
  }
  if (!MIDDLE_reading) {
    MIDDLE_pressCount = 0;
    MIDDLE_state = 0;
  }
  if (!RIGHT_reading) {
    RIGHT_pressCount = 0;
    RIGHT_state = 0;
  }
  if (!SAVE_reading) {
    SAVE_pressCount = 0;
    SAVE_state = 0;
  }
  
  LEFT_lastState = LEFT_state;
  LEFT_lastReading = LEFT_reading;
  MIDDLE_lastState = MIDDLE_state;
  MIDDLE_lastReading = MIDDLE_reading;
  RIGHT_lastState = RIGHT_state;
  RIGHT_lastReading = RIGHT_reading;
  SAVE_lastState = SAVE_state;
  SAVE_lastReading = SAVE_reading;
}

/*
 * Updates the state of the executing program based on the current 
 * state and the button pressed by the user.
 */
void updateState(int button) {
  switch (programState) {
    case STATE_MAIN:
      if (button == LEFT_BUTTON) {
        programState = STATE_LEFT_DETAILS;
      } else if (button == MIDDLE_BUTTON) {
        programState = STATE_OFF;
      } else if (button == RIGHT_BUTTON) {
        programState = STATE_RIGHT_DETAILS;
      }
      break;
      
    case STATE_OFF:
      if (button == LEFT_BUTTON || button == MIDDLE_BUTTON || button == RIGHT_BUTTON) {
        programState = STATE_MAIN;
      }
      break;
      
    case STATE_LEFT_DETAILS:
      if (button == LEFT_BUTTON) {
        programState = STATE_MAIN;
      } else if (button == MIDDLE_BUTTON) {
        programState = STATE_LEFT_SELECT;
      } else if (button == RIGHT_BUTTON) {
        programState = STATE_LEFT_RESET_CONFIRM;
      }
      break;
      
    case STATE_LEFT_SELECT:
      if (button == LEFT_BUTTON) {
        if (beerSelectIndex >= 1) {
          beerSelectIndex--;
        }
      } else if (button == MIDDLE_BUTTON) {
        programState = STATE_LEFT_NEW_CONFIRM;
      } else if (button == RIGHT_BUTTON) {
        if (beerSelectIndex <= BEER_LIST_LENGTH -2) {
          beerSelectIndex++;
        }
      }
      break;
      
    case STATE_LEFT_NEW_CONFIRM:
      if (button == LEFT_BUTTON) {
        leftTapSensor.setPulses(0);
        LEFT_beerCurrentIndex = beerSelectIndex;
        programState = STATE_LEFT_DETAILS;
      } else if (button == MIDDLE_BUTTON) {
        programState = STATE_LEFT_DETAILS;
      } else if (button == RIGHT_BUTTON) {
        programState = STATE_LEFT_SELECT;
      }
      break;
      
    case STATE_LEFT_RESET_CONFIRM:
      if (button == LEFT_BUTTON) {
        leftTapSensor.setPulses(0);
        programState = STATE_LEFT_DETAILS;
      } else if (button == MIDDLE_BUTTON) {
      } else if (button == RIGHT_BUTTON) {
        programState = STATE_LEFT_DETAILS;
      }
      break;
      
    case STATE_RIGHT_DETAILS:
      if (button == LEFT_BUTTON) {
        programState = STATE_RIGHT_RESET_CONFIRM;
      } else if (button == MIDDLE_BUTTON) {
        programState = STATE_RIGHT_SELECT;
      } else if (button == RIGHT_BUTTON) {
        programState = STATE_MAIN;
      }
      break;
      
    case STATE_RIGHT_SELECT:
      if (button == LEFT_BUTTON) {
        if (beerSelectIndex >= 1) {
          beerSelectIndex--;
        }
      } else if (button == MIDDLE_BUTTON) {
        programState = STATE_RIGHT_NEW_CONFIRM;
      } else if (button == RIGHT_BUTTON) {
        if (beerSelectIndex <= BEER_LIST_LENGTH -2) {
          beerSelectIndex++;
        }
      }
      break;
      
    case STATE_RIGHT_NEW_CONFIRM:
      if (button == LEFT_BUTTON) {
        rightTapSensor.setPulses(0);
        RIGHT_beerCurrentIndex = beerSelectIndex;
        programState = STATE_RIGHT_DETAILS;
      } else if (button == MIDDLE_BUTTON) {
        programState = STATE_RIGHT_DETAILS;
      } else if (button == RIGHT_BUTTON) {
        programState = STATE_RIGHT_SELECT;
      }
      break;
      
    case STATE_RIGHT_RESET_CONFIRM:
      if (button == LEFT_BUTTON) {
        rightTapSensor.setPulses(0);
        programState = STATE_RIGHT_DETAILS;
      } else if (button == MIDDLE_BUTTON) {
      } else if (button == RIGHT_BUTTON) {
        programState = STATE_RIGHT_DETAILS;
      }
      break;
      
    default:
      break;
  }
  change_LCD();
}

/*
 * Saves pertinent program state information to non volatile memory.
 */
void saveStateInformation() {
  // Show a message that the state is being saved
  lcd.clear();
  
  // delay to prevent the clear function from causing problems
  delay(100);
  
  lcd.setCursor(3,1);
  lcd.print("Saving sensor");
  lcd.setCursor(3,2);
  lcd.print("state");
  
  int i;
  for (i=0; i<5; i++) {
    lcd.setCursor(9,2);
    lcd.print(".");
    delay(200);
    lcd.print(".");
    delay(200);
    lcd.print(".");
    delay(200);
    lcd.setCursor(9,2);
    lcd.print("   ");
  }
  
  // Perform the saving of the information
  config.leftIndex = LEFT_beerCurrentIndex;
  config.rightIndex = RIGHT_beerCurrentIndex;
  config.leftPulses = leftTapSensor.getPulses();
  config.rightPulses = rightTapSensor.getPulses();
  EEPROM_writeAnything(EE_ADDR_CONFIG, config);
  
  // Change the LCD back
  change_LCD();
}

/*
 * Reads program state information from non volatile memory
 */
void readStateInformation() {
  // Show a message that the state is being read
  lcd.clear();
  
  // delay to prevent the clear function from causing problems
  delay(100);
  
  lcd.setCursor(0,0);
  lcd.print("Welcome to");
  lcd.setCursor(6,1);
  lcd.print("Keg Flowduino");
  lcd.setCursor(0,3);
  lcd.print("Reading Sensor State");
  
  // Perform the reading of the information
  EEPROM_readAnything(EE_ADDR_CONFIG, config);
  LEFT_beerCurrentIndex = config.leftIndex;
  RIGHT_beerCurrentIndex = config.rightIndex;
  leftTapSensor.setPulses(config.leftPulses);
  rightTapSensor.setPulses(config.rightPulses);
  
  // Pause execution for 3 seconds
  delay(3000);
}

/////////////////////////////////////////////////////////////////////////////
//
// Functions for controlling LCD.
//
/////////////////////////////////////////////////////////////////////////////

/*
 * Sets the text of the LCD to match the state.
 */
void change_LCD() {
  
  // Check if the backlight should be turned on/off
  if (programState == STATE_OFF) {
    digitalWrite(BACKLIGHT, LOW);
  } else {
    digitalWrite(BACKLIGHT, HIGH);
  }
  
  lcdCount = 0;
  lcd.clear();
  
  // delay to prevent the clear function from causing problems
  delay(100);
    
  switch (programState) {
    case STATE_LEFT_DETAILS:
      lcd.setCursor(0,0);
      lcd.print(BEER_LIST[LEFT_beerCurrentIndex]);
      lcd.setCursor(0,1);
      lcd.print("Beers Poured:");
      printFloatXX_X(leftTapSensor.getBeersPoured());
      lcd.print("/");
      printFloatXX(leftTapSensor.getMaxBeers());
      lcd.setCursor(0,2);
      lcd.print("Liters Poured: ");
      printFloatXX_XX(leftTapSensor.getLitersPoured());
      lcd.setCursor(0,3);
      lcd.print("Back  New Beer Reset");
      break;
        
    case STATE_LEFT_SELECT:
      if (beerSelectIndex > 0) {
        lcd.setCursor(0,0);
        lcd.print(" ");
        lcd.print(BEER_LIST[beerSelectIndex - 1]);
      }
      if (beerSelectIndex < (BEER_LIST_LENGTH - 1)) {
        lcd.setCursor(0,2);
        lcd.print(" ");
        lcd.print(BEER_LIST[beerSelectIndex + 1]);
      }
      lcd.setCursor(0,1);
      lcd.print("*");
      lcd.print(BEER_LIST[beerSelectIndex]);
      lcd.setCursor(0,3);
      lcd.print(" Up    Select   Down");
      break;
        
    case STATE_LEFT_NEW_CONFIRM:
      lcd.setCursor(0,0);
      lcd.print("Confirm:");
      lcd.setCursor(0,1);
      lcd.print("Reset left to:");
      lcd.setCursor(0,2);
      lcd.print(BEER_LIST[beerSelectIndex]);
      lcd.setCursor(0,3);
      lcd.print(" Yes   Cancel    No ");
      break;
        
    case STATE_LEFT_RESET_CONFIRM:
      lcd.setCursor(0,0);
      lcd.print("Confirm:");
      lcd.setCursor(0,1);
      lcd.print("Reset left to full:");
      lcd.setCursor(0,2);
      lcd.print(BEER_LIST[beerSelectIndex]);
      lcd.setCursor(0,3);
      lcd.print(" Yes             No ");
      break;
        
    case STATE_RIGHT_DETAILS:
      lcd.setCursor(0,0);
      lcd.print(BEER_LIST[RIGHT_beerCurrentIndex]);
      lcd.setCursor(0,1);
      lcd.print("Beers Poured:");
      printFloatXX_X(rightTapSensor.getBeersPoured());
      lcd.print("/");
      printFloatXX(rightTapSensor.getMaxBeers());
      lcd.setCursor(0,2);
      lcd.print("Liters Poured: ");
      printFloatXX_XX(rightTapSensor.getLitersPoured());
      lcd.setCursor(0,3);
      lcd.print("Reset New Beer  Back");
      break;
        
    case STATE_RIGHT_SELECT:
      if (beerSelectIndex > 0) {
        lcd.setCursor(0,0);
        lcd.print(" ");
        lcd.print(BEER_LIST[beerSelectIndex - 1]);
      }
      if (beerSelectIndex < (BEER_LIST_LENGTH - 1)) {
        lcd.setCursor(0,2);
        lcd.print(" ");
        lcd.print(BEER_LIST[beerSelectIndex + 1]);
      }
      lcd.setCursor(0,1);
      lcd.print("*");
      lcd.print(BEER_LIST[beerSelectIndex]);
      lcd.setCursor(0,3);
      lcd.print(" Up    Select   Down");
      break;
        
    case STATE_RIGHT_NEW_CONFIRM:
      lcd.setCursor(0,0);
      lcd.print("Confirm:");
      lcd.setCursor(0,1);
      lcd.print("Reset right to:");
      lcd.setCursor(0,2);
      lcd.print(BEER_LIST[beerSelectIndex]);
      lcd.setCursor(0,3);
      lcd.print(" Yes   Cancel    No ");
      break;
        
    case STATE_RIGHT_RESET_CONFIRM:
      lcd.setCursor(0,0);
      lcd.print("Confirm:");
      lcd.setCursor(0,1);
      lcd.print("Reset right to full:");
      lcd.setCursor(0,2);
      lcd.print(BEER_LIST[beerSelectIndex]);
      lcd.setCursor(0,3);
      lcd.print(" Yes             No ");
      break;
        
    case STATE_OFF:
      break;
      
    case STATE_MAIN:        
      // Print first line of LCD
      lcd.setCursor(0,0);
      lcd.print("    Temp: ");
      printFloatXX_X(getTemp());
      lcd.print(" F");
        
      // Print second line of LCD
      lcd.setCursor(0,1);
      printFloatXX_X(leftTapSensor.getBeersLeft());
      lcd.print(" Beers Left ");
      printFloatXX_X(rightTapSensor.getBeersLeft());
        
      // Print third line of LCD
      lcd.setCursor(0,2);
      printFloatXXX(leftTapSensor.getPercentLeft());
      lcd.print("%  Keg Left  ");
      printFloatXXX(rightTapSensor.getPercentLeft());
      lcd.print("%");
        
      // Print menu on LCD
      lcd.setCursor(0,3);
      lcd.print("Tap 1  Sleep   Tap 2");
        
      break;
       
    default:
      lcd.setCursor(0,1);
      lcd.print("Oops, something");
      lcd.setCursor(0,2);
      lcd.print("went wrong.");
  }
}

/*
 * Updates the dynamic elements of each page.
 */
void update_LCD() {
  if (lcdCount++ >= LCD_COUNT_MAX) {
    lcdCount = 0;
    
    switch (programState) {
      case STATE_LEFT_DETAILS:
        lcd.setCursor(13,1);
        printFloatXX_X(leftTapSensor.getBeersPoured());
        lcd.print("/");
        printFloatXX(leftTapSensor.getMaxBeers());
        lcd.setCursor(15,2);
        printFloatXX_XX(leftTapSensor.getLitersPoured());
        break;
        
      case STATE_LEFT_SELECT:
        break;
        
      case STATE_LEFT_NEW_CONFIRM:
        break;
        
      case STATE_LEFT_RESET_CONFIRM:
        break;
        
      case STATE_RIGHT_DETAILS:
        lcd.setCursor(13,1);
        printFloatXX_X(rightTapSensor.getBeersPoured());
        lcd.print("/");
        printFloatXX(rightTapSensor.getMaxBeers());
        lcd.setCursor(15,2);
        printFloatXX_XX(rightTapSensor.getLitersPoured());
        break;
        
      case STATE_RIGHT_SELECT:
        break;
        
      case STATE_RIGHT_NEW_CONFIRM:
        break;
        
      case STATE_RIGHT_RESET_CONFIRM:
        break;
        
      case STATE_OFF:
        break;
      
      case STATE_MAIN:
        // Update first line of LCD
        lcd.setCursor(9,0);
        printFloatXX_X(getTemp());
        
        // Update second line of LCD
        lcd.setCursor(0,1);
        printFloatXX_X(leftTapSensor.getBeersLeft());
        lcd.setCursor(16,1);
        printFloatXX_X(rightTapSensor.getBeersLeft());
        
        // Update third line of LCD
        lcd.setCursor(0,2);
        printFloatXXX(leftTapSensor.getPercentLeft());
        lcd.setCursor(16,2);
        printFloatXXX(rightTapSensor.getPercentLeft());
        break;
        
      default:
        break;
    }
  }
}

/*
 * Prints out a float in the format ##.#
 */
void printFloatXX_X(float floatNum) {
  // Break up the float value
  int leftSide = (int) floatNum;
  int rightSide = (int) 10 * (floatNum - leftSide);
  
  // Ensure the left side fits in two digits
  while (leftSide >= 100) {
    leftSide -= 100;
  }
  
  // Determine padding necesary and print the number
  if (leftSide < 10) {
    lcd.print(" ");
  }
  lcd.print(leftSide);
  lcd.print(".");
  lcd.print(rightSide);
}

/*
 * Prints out a float in the format ##.##
 */
void printFloatXX_XX(float floatNum) {
  // Break up the float value
  int leftSide = (int) floatNum;
  int rightSide = (int) 100 * (floatNum - leftSide);
  
  // Ensure the left side fits in two digits
  while (leftSide >= 100) {
    leftSide -= 100;
  }
  
  // Determine padding necesary and print the number
  if (leftSide < 10) {
    lcd.print(" ");
  }
  lcd.print(leftSide);
  lcd.print(".");
  lcd.print(rightSide);
}

/*
 * Prints out a float in the format ###
 */
void printFloatXXX(float floatNum) {
  // Break up the float value
  int leftSide = (int) floatNum;
  
  // Ensure the number fits in 3 digits
  while (leftSide >= 1000) {
    leftSide -= 1000;
  }
  
  // Determine padding necesary and print the number
  if (leftSide < 100) {
    lcd.print(" ");
    if (leftSide < 10) {
      lcd.print(" ");
    }
  }
  lcd.print(leftSide);
}

/*
 * Prints out a float in the format ##
 */
void printFloatXX(float floatNum) {
  // Break up the float value
  int leftSide = (int) floatNum;
  
  // Ensure the number fits in 3 digits
  while (leftSide >= 100) {
    leftSide -= 100;
  }
  
  // Determine padding necesary and print the number
  if (leftSide < 10) {
    lcd.print(" ");
    if (leftSide < 1) {
      lcd.print(" ");
    }
  }
  lcd.print(leftSide);
}

/*
 * Sets the backlight of the LCD.
 */
void setBacklight(uint8_t r, uint8_t g, uint8_t b) {
  // normalize the red LED - its brighter than the rest!
  r = map(r, 0, 255, 0, 100);
  g = map(g, 0, 255, 0, 150);
 
  r = map(r, 0, 255, 0, brightness);
  g = map(g, 0, 255, 0, brightness);
  b = map(b, 0, 255, 0, brightness);
 
  // common anode so invert!
  r = map(r, 0, 255, 255, 0);
  g = map(g, 0, 255, 255, 0);
  b = map(b, 0, 255, 255, 0);
  Serial.print("R = "); Serial.print(r, DEC);
  Serial.print(" G = "); Serial.print(g, DEC);
  Serial.print(" B = "); Serial.println(b, DEC);
  analogWrite(REDLITE, r);
  analogWrite(GREENLITE, g);
  analogWrite(BLUELITE, b);
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
    
  // Handle set backlight command
  } else if (command[0] == 'B' && command[1] == 'C') {
    char color[3];
    color[0] = commandBuffer[3];
    color[1] = commandBuffer[4];
    color[2] = commandBuffer[5];
    uint8_t red = atoi(color);
    
    color[0] = commandBuffer[7];
    color[1] = commandBuffer[8];
    color[2] = commandBuffer[9];
    uint8_t green = atoi(color);
    
    color[0] = commandBuffer[11];
    color[1] = commandBuffer[12];
    color[2] = commandBuffer[13];
    uint8_t blue = atoi(color);
    
    setBacklight(red, green, blue);
    
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
    
  // Handle set beer name
  } else if (command[0] == 'S' && command[1] == 'B') {
    uint8_t i;
    if (commandBuffer[3] == 'L') {
      LEFT_beerCurrentIndex = BEER_LIST_LENGTH -2;
      i = BEER_LIST_LENGTH -2;
    } else {
      RIGHT_beerCurrentIndex = BEER_LIST_LENGTH -1;
      i = BEER_LIST_LENGTH -1;
    }
    
    BEER_LIST[i][0] = commandBuffer[5];
    BEER_LIST[i][1] = commandBuffer[6];
    BEER_LIST[i][2] = commandBuffer[7];
    BEER_LIST[i][3] = commandBuffer[8];
    BEER_LIST[i][4] = commandBuffer[9];
    BEER_LIST[i][5] = commandBuffer[10];
    BEER_LIST[i][6] = commandBuffer[11];
    BEER_LIST[i][7] = commandBuffer[12];
    BEER_LIST[i][8] = commandBuffer[13];
    BEER_LIST[i][9] = commandBuffer[14];
    BEER_LIST[i][10] = commandBuffer[15];
    BEER_LIST[i][11] = commandBuffer[16];
    BEER_LIST[i][12] = commandBuffer[17];
    BEER_LIST[i][13] = commandBuffer[18];
    BEER_LIST[i][14] = commandBuffer[19];
    BEER_LIST[i][15] = commandBuffer[20];
    BEER_LIST[i][16] = commandBuffer[21];
    BEER_LIST[i][17] = commandBuffer[22];
    BEER_LIST[i][18] = commandBuffer[23];
  
  // Handle get keg level (pulses)
  } else if (command[0] == 'G' && command[1] == 'P') {
    if (commandBuffer[3] == 'L') {
      Serial.println(leftTapSensor.getPulses());
    } else {
      Serial.println(rightTapSensor.getPulses());
    }
    
  // Handle get beer name
  } else if (command[0] == 'G' && command[1] == 'B') {
    if (commandBuffer[3] == 'L') {
      Serial.println(BEER_LIST[LEFT_beerCurrentIndex]);
    } else {
      Serial.println(BEER_LIST[RIGHT_beerCurrentIndex]);
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

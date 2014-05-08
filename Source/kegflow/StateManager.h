#ifndef StateManager_h
#define StateManager_h

#include "Arduino.h"
#include "EEPROMAnything.h"

// Define a struct to hold program state data for saving to and reading from non volatile memory
typedef struct {
  uint16_t leftPulses;
  uint16_t rightPulses;
} config_t;

class StateManager {
  private:
    int address;
    
  public:
              StateManager(int newAddress);
             ~StateManager();
    boolean   loadState(config_t configData);
    boolean   saveState(uint16_t newLeftPulses, uint16_t newRightPulses);
};

#endif

#include "StateManager.h"

StateManager::StateManager(int newAddress) {
  address = newAddress;
}

StateManager::~StateManager() {
}

boolean StateManager::loadState(config_t configData) {
  EEPROM_readAnything(address, configData);
}

boolean StateManager::saveState(uint16_t newLeftPulses, uint16_t newRightPulses) {
  config_t data;
  data.leftPulses = newLeftPulses;
  data.rightPulses = newRightPulses;
  int bytesWritten = EEPROM_writeAnything(address, data);
  return bytesWritten != 0;
}

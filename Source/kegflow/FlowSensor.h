#ifndef FlowSensor_h
#define FlowSensor_h

#include "Arduino.h"

#define MAX_PULSES 6737.8746353
#define PULSES_PER_BEER 127.1297101

class FlowSensor {
  private:
    uint8_t   pin;
    
    uint16_t  pulses;
    uint8_t   lastflowpinstate;
    uint32_t  lastflowratetimer;
    float     flowrate;
    
  public:
              FlowSensor();
             ~FlowSensor();
    void      init(uint8_t newPin);
    float     getFlowRate();
    uint16_t  getPulses();
    void      setPulses(uint16_t newPulses);
    float     getLiters();
    void      checkSensor();
    float     getBeersLeft();
    float     getPercentLeft();
    float     getBeersPoured();
    float     getLitersPoured();
    float     getMaxBeers();
};

#endif

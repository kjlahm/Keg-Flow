#ifndef FlowSensor_h
#define FlowSensor_h

#include "Arduino.h"

#define MAX_PULSES 7350.408695
#define PULSES_PER_BEER 138.6869565

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
    void      setBeersLeft(float beers);
    float     getBeersLeft();
    float     getPercentLeft();
    float     getBeersPoured();
    float     getLitersPoured();
    float     getMaxBeers();
};

#endif

#include "FlowSensor.h"

FlowSensor::FlowSensor() {
  pulses = 0;
  lastflowratetimer = 0;
}

FlowSensor::~FlowSensor() {
}

void FlowSensor::init(uint8_t newPin) {
  pin = newPin;
  
  pinMode(pin, INPUT);
  digitalWrite(pin, HIGH);
  
  lastflowpinstate = digitalRead(pin);
}

float FlowSensor::getFlowRate() {
  return flowrate;
}

uint16_t FlowSensor::getPulses() {
  return pulses;
}

void FlowSensor::setPulses(uint16_t newPulses) {
  pulses = newPulses;
}

float FlowSensor::getLiters() {
  // if a plastic sensor use the following calculation
  // Sensor Frequency (Hz) = 7.5 * Q (Liters/min)
  // Liters = Q * time elapsed (seconds) / 60 (seconds/minute)
  // Liters = (Frequency (Pulses/second) / 7.5) * time elapsed (seconds) / 60
  // Liters = Pulses / (7.5 * 60)
  float liters = pulses;
  liters /= 7.5;
  liters /= 60.0;
  return liters;
}

void FlowSensor::checkSensor() {
  uint8_t x = digitalRead(pin);
  
  if (x == lastflowpinstate) {
    lastflowratetimer++;
    return; // nothing changed
  }
  
  if (x == HIGH) {
    // low to high transition
    pulses++;
  }
  lastflowpinstate = x;
  flowrate = 1000.0;
  flowrate /= lastflowratetimer;  // in hertz
  lastflowratetimer = 0;
}

float FlowSensor::getBeersLeft() {
  float beersLeft = MAX_PULSES - pulses;
  if (beersLeft < 0.0) {
    beersLeft = 0.0;
  } else {
    beersLeft = beersLeft / PULSES_PER_BEER;
  }
  return beersLeft;
}

float FlowSensor::getPercentLeft() {
  float percentLeft = MAX_PULSES - pulses;
  if (percentLeft < 0.0) {
    percentLeft = 0.0;
  } else {
    percentLeft = percentLeft / MAX_PULSES;
  }
  return percentLeft * 100;
}

float FlowSensor::getBeersPoured() {
  return pulses / PULSES_PER_BEER;
}

float FlowSensor::getLitersPoured() {
  float ounces = (pulses / PULSES_PER_BEER) * 12;
  return ounces * 33.814;
}

float FlowSensor::getMaxBeers() {
  return MAX_PULSES / PULSES_PER_BEER;
}

#pragma once
// #define MEMORY_FUNCTIONS

struct my_pin_config {
  static constexpr int powerSwitch = 2;
  static constexpr int modeSwitch = 3;
  static constexpr int displayButton = 8;
  static constexpr int myEncoder[2] = {6, 7};
  static constexpr int tempSensor = 5;
  static constexpr int relais = 4;
};


struct config {
  static constexpr int encoderValCutoff = 6;
  static constexpr float tempStep = 0.5; 
  static constexpr int tempMax = 30; 
  static constexpr int tempMin = 5; 
  static constexpr float tolerance = 1.5;
};


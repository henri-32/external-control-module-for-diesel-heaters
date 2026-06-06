#pragma once

//#define MEMORY_FUNCTIONS

struct PinConfig {
  static constexpr int kPowerSwitchPin = 2;
  static constexpr int kModeSwitchPin = 3;
  static constexpr int kDisplayButtonPin = 8;
  static constexpr int kEncoderPinA = 6;
  static constexpr int kEncoderPinB = 7;
  static constexpr int kTempSensorPin = 5;
  static constexpr int kRelaisPin = 4;
};

struct DebounceConfig {
  static constexpr int kEncoderMs = 100; 
  static constexpr unsigned long kPushButtonMs = 50; 
  static constexpr unsigned long kToggleSwitchMs = 50; 
}; 

struct Config {
  static constexpr unsigned long kTemperatureRequestIntervalMs = 2000; 
  static constexpr float kDefaultTempC = 15; 
  static constexpr int kEncoderValCutoff = 6;
  static constexpr float kTempStepC = 0.5; 
  static constexpr int kTempMaxC = 30; 
  static constexpr int kTempMinC = 5; 
  static constexpr float kToleranceC = 1.5;
  static constexpr int kRelaisLongPulseMs = 1500; 
  static constexpr int kRelaisShortPulseMs = 200; 
  static constexpr int kMinLcdUpdateIntervalMs = 100; 
};


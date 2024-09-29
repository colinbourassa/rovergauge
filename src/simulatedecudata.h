#pragma once
#include <cstdint>
#include "comm14cux.h"

class SimulatedECUData
{
public:
  SimulatedECUData();
  float maf();
  float throttle();
  int16_t lambdaShortOdd();
  int16_t lambdaShortEven();
  int16_t lambdaLongOdd();
  int16_t lambdaLongEven();
  bool mil();
  float coolantTempF();
  float fuelTempF();
  float mainVoltage();
  float coTrimVoltage();
  uint16_t engineRPM();
  uint16_t engineRPMLimit();
  void engineRPMTable(c14cux_rpmtable& table);
  void fuelMapData(uint8_t* buf, uint16_t& mafScaler, uint16_t& adjFactor);
  void fuelMapRowColIndices(uint8_t& rowIndex, uint8_t& rowWeight, uint8_t& colIndex, uint8_t& colWeight);
  uint16_t targetIdle();
  bool idleMode();
  uint8_t currentFuelMap();
  uint8_t roadSpeedMPH();
  float idleBypassPos();
  bool fuelPumpRelayState();
  uint8_t gearSelection();
  uint16_t injectorPulsewidthUs();

private:
  float m_maf = 0.0;
  bool m_mafDirection = true;
  float m_throttle = 0.0;
  bool m_throttleDirection = true;
  int m_lambdaShortOdd = 0;
  bool m_lambdaShortOddDirection = true;
  int m_lambdaShortEven = 0;
  bool m_lambdaShortEvenDirection = false;
  int m_lambdaLongOdd = 0;
  bool m_lambdaLongOddDirection = true;
  int m_lambdaLongEven = 0;
  bool m_lambdaLongEvenDirection = false;
  bool m_milOn = false;
  float m_coolantTempF = 40.0f;
  bool m_coolantTempDirection = true;
  float m_fuelTempF = 40.0f;
  bool m_fuelTempDirection = true;
  float m_mainVoltage = 12.0f;
  bool m_mainVoltageDirection = true;
  float m_coTrimVoltage = 4.0f;
  bool m_coTrimVoltageDirection = true;
  int m_engineRPM = 750;
  bool m_engineRPMDirection = true;
  int m_engineRPMLimit = 5750;
  int m_fuelMapRowIndex = 0;
  int m_fuelMapRowWeight = 0;
  bool m_fuelMapRowDirection = true;
  int m_fuelMapColumnIndex = 0;
  int m_fuelMapColumnWeight = 0;
  bool m_fuelMapColumnDirection = true;
  int m_targetIdleRPM = 580;
  bool m_targetIdleRPMDirection = true;
  int m_currentFuelMap = 5;
  int m_roadSpeedMPH = 0;
  bool m_roadSpeedMPHDirection = true;
  float m_idleBypassPercentage = 0.4f;
  bool m_idleBypassPercentageDirection = false;
  int m_injectorPulseWidthUs = 100;
  bool m_injectorPulseWidthUsDirection = true;

  void adjust(int& val, bool& direction, int min, int max, int inc);
  void adjust(float& val, bool& direction, float min, float max, float inc);
  void adjustFuelMapRowCol();
};


#pragma once
#include <stdint.h>
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
  float m_maf;
  bool m_mafDirection;
  float m_throttle;
  bool m_throttleDirection;
  int m_lambdaShortOdd;
  bool m_lambdaShortOddDirection;
  int m_lambdaShortEven;
  bool m_lambdaShortEvenDirection;
  int m_lambdaLongOdd;
  bool m_lambdaLongOddDirection;
  int m_lambdaLongEven;
  bool m_lambdaLongEvenDirection;
  bool m_milOn;
  float m_coolantTempF;
  bool m_coolantTempDirection;
  float m_fuelTempF;
  bool m_fuelTempDirection;
  float m_mainVoltage;
  bool m_mainVoltageDirection;
  float m_coTrimVoltage;
  bool m_coTrimVoltageDirection;
  int m_engineRPM;
  bool m_engineRPMDirection;
  int m_engineRPMLimit;
  int m_fuelMapRowIndex;
  int m_fuelMapRowWeight;
  bool m_fuelMapRowDirection;
  int m_fuelMapColumnIndex;
  int m_fuelMapColumnWeight;
  bool m_fuelMapColumnDirection;
  bool m_fuelMapColumnWeightDirection;
  int m_targetIdleRPM;
  bool m_targetIdleRPMDirection;
  int m_currentFuelMap;
  int m_roadSpeedMPH;
  bool m_roadSpeedMPHDirection;
  float m_idleBypassPercentage;
  bool m_idleBypassPercentageDirection;
  int m_injectorPulseWidthUs;
  bool m_injectorPulseWidthUsDirection;

  void adjust(int& val, bool& direction, int min, int max, int inc);
  void adjust(float& val, bool& direction, float min, float max, float inc);
  void adjustFuelMapRowCol();
};

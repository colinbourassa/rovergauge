#include "simulatedecudata.h"

SimulatedECUData::SimulatedECUData() :
  m_maf(0.0),
  m_mafDirection(true),
  m_throttle(0.0),
  m_throttleDirection(true),
  m_lambdaShortOdd(0),
  m_lambdaShortOddDirection(true),
  m_lambdaShortEven(0),
  m_lambdaShortEvenDirection(false),
  m_lambdaLongOdd(0),
  m_lambdaLongOddDirection(true),
  m_lambdaLongEven(0),
  m_lambdaLongEvenDirection(false),
  m_milOn(false),
  m_coolantTempF(40.0f),
  m_coolantTempDirection(true),
  m_fuelTempF(40.0f),
  m_fuelTempDirection(true),
  m_mainVoltage(12.0f),
  m_mainVoltageDirection(true),
  m_coTrimVoltage(4.0f),
  m_coTrimVoltageDirection(true),
  m_engineRPM(750),
  m_engineRPMDirection(true),
  m_engineRPMLimit(5750),
  m_fuelMapRowIndex(0),
  m_fuelMapRowIndexDirection(true),
  m_fuelMapRowWeight(0),
  m_fuelMapRowWeightDirection(true),
  m_fuelMapColumnIndex(0),
  m_fuelMapColumnIndexDirection(true),
  m_fuelMapColumnWeight(0),
  m_fuelMapColumnWeightDirection(true),
  m_targetIdleRPM(580),
  m_targetIdleRPMDirection(true),
  m_currentFuelMap(5),
  m_roadSpeedMPH(0),
  m_idleBypassPercentage(0.4f),
  m_idleBypassPercentageDirection(false),
  m_injectorPulseWidthUs(100),
  m_injectorPulseWidthUsDirection(true)
{
}

void SimulatedECUData::adjust(int& val, bool& direction, int min, int max, int increment)
{
  if (direction)
  {
    val += increment;
  }
  else
  {
    val -= increment;
  }

  if (direction && (val == max))
  {
    direction = false;
  }
  else if (!direction && (val == min))
  {
    direction = true;
  }
}

void SimulatedECUData::adjust(float& val, bool& direction, float min, float max, float increment)
{
  if (direction)
  {
    val += increment;
  }
  else
  {
    val -= increment;
  }

  if (direction && (val >= max))
  {
    direction = false;
  }
  else if (!direction && (val <= min))
  {
    direction = true;
  }
}

float SimulatedECUData::maf()
{
  adjust(m_maf, m_mafDirection, 0.01f, 0.99f, 0.01f);
  return m_maf;
}

float SimulatedECUData::throttle()
{
  adjust(m_throttle, m_throttleDirection, 0.01f, 0.99f, 0.01f);
  return m_throttle;
}

int16_t SimulatedECUData::lambdaShortOdd()
{
  adjust(m_lambdaShortOdd, m_lambdaShortOddDirection, -256, 255, 1);
  return m_lambdaShortOdd;
}

int16_t SimulatedECUData::lambdaShortEven()
{
  adjust(m_lambdaShortEven, m_lambdaShortEvenDirection, -256, 255, 1);
  return m_lambdaShortEven;
}

int16_t SimulatedECUData::lambdaLongOdd()
{
  adjust(m_lambdaLongOdd, m_lambdaLongOddDirection, -256, 255, 1);
  return m_lambdaLongOdd;
}

int16_t SimulatedECUData::lambdaLongEven()
{
  adjust(m_lambdaLongEven, m_lambdaLongEvenDirection, -256, 255, 1);
  return m_lambdaLongEven;
}

bool SimulatedECUData::mil()
{
  return m_milOn;
}

float SimulatedECUData::coolantTempF()
{
  adjust(m_coolantTempF, m_coolantTempDirection, 40.0f, 230.0f, 2.5f);
  return m_coolantTempF;
}

float SimulatedECUData::fuelTempF()
{
  adjust(m_fuelTempF, m_fuelTempDirection, 40.0f, 230.0f, 2.0f);
  return m_fuelTempF;
}

float SimulatedECUData::mainVoltage()
{
  adjust(m_mainVoltage, m_mainVoltageDirection, 10.0f, 14.0f, 0.05f);
  return m_mainVoltage;
}

float SimulatedECUData::coTrimVoltage()
{
  adjust(m_coTrimVoltage, m_coTrimVoltageDirection, 0.5f, 4.9f, 0.05f);
  return m_coTrimVoltage;
}

uint16_t SimulatedECUData::engineRPM()
{
  adjust(m_engineRPM, m_engineRPMDirection, 600, 5500, 5);
  return m_engineRPM;
}

uint16_t SimulatedECUData::engineRPMLimit()
{
  return m_engineRPMLimit;
}

void SimulatedECUData::engineRPMTable(c14cux_rpmtable& table)
{
  for (int col = 0; col < FUEL_MAP_COLUMNS; col++)
  {
    table.rpm[col] = 500 + (col * 250);
  }
}

void SimulatedECUData::fuelMapData(uint8_t* buf, uint16_t& mafScaler, uint16_t& adjFactor)
{
  mafScaler = 0xabcd;
  adjFactor = 0xaaaa;
  for (int row = 0; row < FUEL_MAP_ROWS; row++)
  {
    for (int col = 0; col < FUEL_MAP_COLUMNS; col++)
    {
      buf[row*16 + col] = row * col * 2 + row * 3 + col;
    }
  }
}

uint8_t SimulatedECUData::fuelMapRowIndex()
{
  adjust(m_fuelMapRowIndex, m_fuelMapRowIndexDirection, 0, 7, 1);
  return m_fuelMapRowIndex;
}

uint8_t SimulatedECUData::fuelMapRowWeighting()
{
  adjust(m_fuelMapRowWeight, m_fuelMapRowWeightDirection, 0, 15, 1);
  return m_fuelMapRowWeight;
}

uint8_t SimulatedECUData::fuelMapColumnIndex()
{
  adjust(m_fuelMapColumnIndex, m_fuelMapColumnIndexDirection, 0, 15, 1);
  return m_fuelMapColumnIndex;
}

uint8_t SimulatedECUData::fuelMapColumnWeighting()
{
  adjust(m_fuelMapColumnWeight, m_fuelMapColumnWeightDirection, 0, 15, 1);
  return m_fuelMapColumnWeight;
}

uint16_t SimulatedECUData::targetIdle()
{
  adjust(m_targetIdleRPM, m_targetIdleRPMDirection, 580, 700, 3);
  return m_targetIdleRPM;
}

bool SimulatedECUData::idleMode()
{
  return (m_targetIdleRPM < 700) && (m_throttle < 0.05);
}

uint8_t SimulatedECUData::currentFuelMap()
{
  return m_currentFuelMap;
}

uint8_t SimulatedECUData::roadSpeedMPH()
{
  adjust(m_roadSpeedMPH, m_roadSpeedMPHDirection, 20, 25, 1);
  return m_roadSpeedMPH;
}

float SimulatedECUData::idleBypassPos()
{
  adjust(m_idleBypassPercentage, m_idleBypassPercentageDirection, 0.01f, 0.99f, 0.01f);
  return m_idleBypassPercentage;
}

bool SimulatedECUData::fuelPumpRelayState()
{
  return true;
}

uint8_t SimulatedECUData::gearSelection()
{
  return 0x03;
}

uint16_t SimulatedECUData::injectorPulsewidthUs()
{
  adjust(m_injectorPulseWidthUs, m_injectorPulseWidthUsDirection, 100, 20000, 100);
  return m_injectorPulseWidthUs;
}


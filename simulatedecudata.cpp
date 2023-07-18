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
  m_milOn(false)
{

}

float SimulatedECUData::maf()
{
  m_maf += m_mafDirection ? 1.0f : -1.0f;
  m_mafDirection = (m_maf < 100.0f);
  if (m_mafDirection && (m_maf > 99.0f))
  {
    m_mafDirection = false;
  }
  else if (!m_mafDirection && (m_maf < 1.0f))
  {
    m_mafDirection = true;
  }
  return m_maf;
}

float SimulatedECUData::throttle()
{
  m_throttle += m_throttleDirection ? 1.0f : -1.0f;
  if (m_throttleDirection && (m_throttle > 99.0f))
  {
    m_throttleDirection = false;
  }
  else if (!m_throttleDirection && (m_throttle < 1.0f))
  {
    m_throttleDirection = true;
  }
  return m_throttle;
}

int16_t SimulatedECUData::lambdaShortOdd()
{
  m_lambdaShortOdd += m_lambdaShortOddDirection ? 1 : -1;
  if (m_lambdaShortOddDirection && (m_lambdaShortOdd >= 255))
  {
    m_lambdaShortOddDirection = false;
  }
  else if (!m_lambdaShortOddDirection && (m_lambdaShortOdd <= -256))
  {
    m_lambdaShortOddDirection = true;
  }
  return m_lambdaShortOdd;
}

int16_t SimulatedECUData::lambdaShortEven()
{
  m_lambdaShortEven += m_lambdaShortEvenDirection ? 1 : -1;
  if (m_lambdaShortEvenDirection && (m_lambdaShortEven >= 255))
  {
    m_lambdaShortEvenDirection = false;
  }
  else if (!m_lambdaShortEvenDirection && (m_lambdaShortEven <= -256))
  {
    m_lambdaShortEvenDirection = true;
  }
  return m_lambdaShortEven;
}

int16_t SimulatedECUData::lambdaLongOdd()
{
  m_lambdaLongOdd += m_lambdaLongOddDirection ? 1 : -1;
  if (m_lambdaLongOddDirection && (m_lambdaLongOdd >= 255))
  {
    m_lambdaLongOddDirection = false;
  }
  else if (!m_lambdaLongOddDirection && (m_lambdaLongOdd <= -256))
  {
    m_lambdaLongOddDirection = true;
  }
  return m_lambdaLongOdd;
}

int16_t SimulatedECUData::lambdaLongEven()
{
  m_lambdaLongEven += m_lambdaLongEvenDirection ? 1 : -1;
  if (m_lambdaLongEvenDirection && (m_lambdaLongEven >= 255))
  {
    m_lambdaLongEvenDirection = false;
  }
  else if (!m_lambdaLongEvenDirection && (m_lambdaLongEven <= -256))
  {
    m_lambdaLongEvenDirection = true;
  }
  return m_lambdaLongEven;
}

bool SimulatedECUData::mil()
{
  return m_milOn;
}


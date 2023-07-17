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
  m_lambdaLongEvenDirection(false)
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
  m_throttleDirection = (m_throttle < 100.0f);
  return m_throttle;
}

int16_t SimulatedECUData::lambdaShortOdd()
{
  m_lambdaShortOdd += m_lambdaShortOddDirection ? 1 : -1;
  m_lambdaShortOdd = (m_throttle < 100.0f);
  return m_throttle;
}



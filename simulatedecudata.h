#ifndef SIMULATEDECUDATA_H
#define SIMULATEDECUDATA_H

#include <stdint.h>

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

private:
  float m_maf;
  bool m_mafDirection;
  float m_throttle;
  bool m_throttleDirection;
  int16_t m_lambdaShortOdd;
  bool m_lambdaShortOddDirection;
  int16_t m_lambdaShortEven;
  bool m_lambdaShortEvenDirection;
  int16_t m_lambdaLongOdd;
  bool m_lambdaLongOddDirection;
  int16_t m_lambdaLongEven;
  bool m_lambdaLongEvenDirection;
  bool m_milOn;
  float m_coolantTempF;
  bool m_coolantTempDirection;
  float m_fuelTempF;
  bool m_fuelTempDirection;
};

#endif // SIMULATEDECUDATA_H

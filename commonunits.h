#ifndef COMMONUNITS_H
#define COMMONUNITS_H

#include <stdint.h>

enum SpeedUnits
{
  MPH,
  KPH
};

enum TemperatureUnits
{
  Fahrenheit,
  Celsius
};

enum SampleType
{
  SampleType_EngineTemperature,
  SampleType_RoadSpeed,
  SampleType_EngineRPM,
  SampleType_FuelTemperature,
  SampleType_MAF,
  SampleType_Throttle,
  SampleType_IdleBypassPosition,
  SampleType_TargetIdleRPM,
  SampleType_GearSelection,
  SampleType_MainVoltage,
  SampleType_LambdaTrimShort,
  SampleType_LambdaTrimLong,
  SampleType_COTrimVoltage,
  SampleType_FuelPumpRelay,
  SampleType_FuelMapRowCol,
  SampleType_FuelMapData,
  SampleType_FuelMapIndex,
  SampleType_InjectorPulseWidth,
  SampleType_MIL,
  SampleType_NumSampleTypes
};

typedef struct
{
  uint8_t airConLoad;
  uint16_t maf;
  uint16_t mafTrim;
  uint16_t throttle;
  uint8_t coolantTemp;
  uint8_t fuelTemp;
  uint8_t o2SensorReference;
  uint8_t mainRelay;
  uint8_t inertiaSwitch;
  uint8_t neutralSwitch;
  uint8_t heatedScreen;
  uint8_t diagnosticPlug;
  uint8_t tuneResistor;
  uint8_t o2OddDutyCycle;
  uint8_t o2EvenDutyCycle;

} SimulationInputValues;

typedef struct
{
  bool airConLoad;
  bool maf;
  bool mafTrim;
  bool throttle;
  bool coolantTemp;
  bool fuelTemp;
  bool o2SensorReference;
  bool mainRelay;
  bool inertiaSwitch;
  bool neutralSwitch;
  bool heatedScreen;
  bool diagnosticPlug;
  bool tuneResistor;
  bool o2OddDutyCycle;
  bool o2EvenDutyCycle;

} SimulationInputChanges;

#endif // COMMONUNITS_H

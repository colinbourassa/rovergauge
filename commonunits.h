#ifndef COMMONUNITS_H
#define COMMONUNITS_H

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

enum QueueableRequest
{
  QueueableRequest_ROMImage,
  QueueableRequest_FuelMapData,
  QueueableRequest_RPMTable,
  QueueableRequest_FuelPumpRun,
  QueueableRequest_IACMotorDrive,
  QueueableRequest_TuneRevID,
  QueueableRequest_FaultCodes,
  QueueableRequest_ClearFaultCodes,
  QueueableRequest_BatteryBackedMem
};

#endif // COMMONUNITS_H

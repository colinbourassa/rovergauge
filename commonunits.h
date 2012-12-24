#ifndef COMMONUNITS_H
#define COMMONUNITS_H

enum SpeedUnits
{
    MPH = 0,
    FPS = 1,
    KPH = 2
};

enum TemperatureUnits
{
    Fahrenheit = 0,
    Celcius    = 1
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
    SampleType_LambdaTrim,
    SampleType_FuelMap,
    SampleType_FuelPumpRelay
};

#endif // COMMONUNITS_H

#ifndef CUXINTERFACE_H
#define CUXINTERFACE_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QHash>
#include <QByteArray>
#include <QHash>
#include "comm14cux.h"
#include "commonunits.h"

class CUXInterface : public QObject
{
    enum ReadResult
    {
        ReadResult_Success,
        ReadResult_Failure,
        ReadResult_NoStatement
    };

    Q_OBJECT
public:
    explicit CUXInterface(QString device, SpeedUnits sUnits,
                          TemperatureUnits tUnits, QObject *parent = 0);
    ~CUXInterface();

    void setSerialDevice(QString device);
    void setLambdaTrimType(int type);
    void setMAFReadingType(Comm14CUXAirflowType type);
    void setThrottleReadingType(Comm14CUXThrottlePosType type);
    void setEnabledSamples(QHash<SampleType,bool> samples);

    QString getSerialDevice();
    int getIntervalMsecs();

    bool isConnected();
    void disconnectFromECU();

    int getRoadSpeed();
    int getEngineSpeedRPM();
    int getTargetIdleSpeed();
    int getCoolantTemp();
    int getFuelTemp();
    float getThrottlePos();
    Comm14CUXGear getGear();
    Comm14CUXFaultCodes getFaultCodes();
    float getMainVoltage();
    Comm14CUXVersion getVersion();
    QByteArray* getFuelMap(int fuelMapId);
    int getFuelMapAdjustmentFactor();
    int getCurrentFuelMapIndex();
    int getFuelMapRowIndex();
    int getFuelMapColumnIndex();
    float getMAFReading();
    float getIdleBypassPos();
    bool getFuelPumpRelayState();
    QByteArray* getPROMImage();
    int getLeftLambdaTrim();
    int getRightLambdaTrim();
    bool isMILOn();

    void setSpeedUnits(SpeedUnits units);
    void setTemperatureUnits(TemperatureUnits units);

    void cancelRead();

public slots:
    void onParentThreadStarted();
    void onParentThreadFinished();
    void onFaultCodesRequested();
    void onFaultCodesClearRequested();
    void onFuelMapRequested(int fuelMapId);
    void onReadPROMImageRequested();
    void onStartPollingRequest();
    void onShutdownThreadRequest();
    void onFuelPumpRunRequest();
    void onIdleAirControlMovementRequest(int direction, int steps);

signals:
    void dataReady();
    void connected();
    void disconnected();
    void readError();
    void readSuccess();
    void faultCodesReady();
    void faultCodesReadFailed();
    void faultCodesClearSuccess(Comm14CUXFaultCodes faultCodes);
    void faultCodesClearFailure();
    void fuelMapReady(int fuelMapId);
    void fuelMapReadFailed(int fuelMapId);
    void rpmLimitReady(int rpmLimiter);
    void revisionNumberReady(int tuneRevisionNum);
    void promImageReady();
    void promImageReadFailed();
    void failedToConnect(QString dev);
    void interfaceReadyForPolling();
    void notConnected();

private slots:
    void onTimer();

private:
    int intervalMsecs;
    QString deviceName;
    Comm14CUX *cux;
    QTimer *timer;
    bool stopPolling;
    bool shutdownThread;
    Comm14CUXFaultCodes faultCodes;
    bool readCanceled;
    unsigned long readCount;
    QHash<SampleType,bool> enabledSamples;

    int lambdaTrimType;
    Comm14CUXAirflowType airflowType;
    Comm14CUXThrottlePosType throttlePosType;

    uint16_t roadSpeedMPH;
    uint16_t engineSpeedRPM;
    uint16_t targetIdleSpeed;
    int16_t coolantTempF;
    int16_t fuelTempF;
    float throttlePos;
    Comm14CUXGear gear;
    float mainVoltage;
    uint8_t currentFuelMapIndex;
    uint8_t currentFuelMapRowIndex;
    uint8_t currentFuelMapColumnIndex;
    float mafReading;
    float idleBypassPos;
    bool fuelPumpRelayOn;
    int16_t leftLambdaTrim;
    int16_t rightLambdaTrim;
    bool milOn;
    uint16_t tuneRevision;
    uint16_t rpmLimit;

    QByteArray *promImage;
    QHash<int, QByteArray*> fuelMaps;
    uint16_t fuelMapAdjFactor;

    SpeedUnits speedUnits;
    TemperatureUnits tempUnits;

    qint64 lastMidFreqReadTime;
    qint64 lastLowFreqReadTime;

    void pollEcu();
    ReadResult readData();
    ReadResult readHighFreqData();
    ReadResult readMidFreqData();
    ReadResult readLowFreqData();
    bool connectToECU();
    int convertSpeed(int speedMph);
    int convertTemperature(int tempF);
    ReadResult mergeResult(ReadResult total, ReadResult single);
    ReadResult mergeResult(ReadResult total, bool single);
};

#endif // CUXINTERFACE_H

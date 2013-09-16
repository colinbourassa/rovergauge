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
    void setLambdaTrimType(c14cux_lambda_trim_type type);
    void setMAFReadingType(c14cux_airflow_type type);
    void setThrottleReadingType(c14cux_throttle_pos_type type);
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
    c14cux_gear getGear();
    c14cux_faultcodes getFaultCodes();
    float getMainVoltage();
    c14cux_version getVersion();
    QByteArray* getFuelMap(int fuelMapId);
    int getFuelMapAdjustmentFactor(int fuelMapId);
    int getCurrentFuelMapIndex();
    int getFuelMapRowIndex();
    int getFuelMapColumnIndex();
    float getMAFReading();
    float getIdleBypassPos();
    bool getFuelPumpRelayState();
    QByteArray* getROMImage();
    int getLeftLambdaTrim();
    int getRightLambdaTrim();
    bool isMILOn();
    bool getIdleMode();

    void setSpeedUnits(SpeedUnits units);
    void setTemperatureUnits(TemperatureUnits units);

    void cancelRead();

public slots:
    void onParentThreadStarted();
    void onParentThreadFinished();
    void onFaultCodesRequested();
    void onFaultCodesClearRequested();
    void onFuelMapRequested(int fuelMapId);
    void onReadROMImageRequested();
    void onStartPollingRequest();
    void onShutdownThreadRequest();
    void onFuelPumpRunRequest();
    void onIdleAirControlMovementRequest(int direction, int steps);
#ifdef ENABLE_SIM_MODE
    void onSimModeWriteRequest(bool enableSimMode, SimulationInputValues simVals, SimulationInputChanges changes);
#endif

signals:
    void dataReady();
    void connected();
    void disconnected();
    void readError();
    void readSuccess();
    void faultCodesReady();
    void faultCodesReadFailed();
    void faultCodesClearSuccess(c14cux_faultcodes m_faultCodes);
    void faultCodesClearFailure();
    void fuelMapReady(int fuelMapId);
    void fuelMapReadFailed(int fuelMapId);
    void rpmLimitReady(int rpmLimiter);
    void revisionNumberReady(int tuneRevisionNum);
    void romImageReady();
    void romImageReadFailed();
    void failedToConnect(QString dev);
    void interfaceReadyForPolling();
    void notConnected();
    void simModeWriteSuccess();
    void simModeWriteFailure();

private slots:
    void onTimer();

private:
    QString m_deviceName;
    c14cux_info m_cuxinfo;
    QTimer *m_timer;
    bool m_stopPolling;
    bool m_shutdownThread;
    c14cux_faultcodes m_faultCodes;
    bool m_readCanceled;
    unsigned long m_readCount;
    QHash<SampleType,bool> m_enabledSamples;

    c14cux_lambda_trim_type m_lambdaTrimType;
    c14cux_airflow_type m_airflowType;
    c14cux_throttle_pos_type m_throttlePosType;

    uint16_t m_roadSpeedMPH;
    uint16_t m_engineSpeedRPM;
    uint16_t m_targetIdleSpeed;
    int16_t m_coolantTempF;
    int16_t m_fuelTempF;
    float m_throttlePos;
    c14cux_gear m_gear;
    float m_mainVoltage;
    uint8_t m_currentFuelMapIndex;
    uint8_t m_currentFuelMapRowIndex;
    uint8_t m_currentFuelMapColumnIndex;
    float m_mafReading;
    float m_idleBypassPos;
    bool m_fuelPumpRelayOn;
    int16_t m_leftLambdaTrim;
    int16_t m_rightLambdaTrim;
    bool m_milOn;
    uint16_t m_tuneRevision;
    uint16_t m_rpmLimit;
    bool m_idleMode;

    QByteArray *m_romImage;
    QHash<int, QByteArray*> m_fuelMaps;
    QHash<int, int> m_fuelMapAdjFactors;

    SpeedUnits m_speedUnits;
    TemperatureUnits m_tempUnits;

    qint64 m_lastMidFreqReadTime;
    qint64 m_lastLowFreqReadTime;

    bool m_initComplete;

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

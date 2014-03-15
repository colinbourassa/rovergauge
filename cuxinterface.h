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

static const unsigned int fuelMapCount = 6;

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
                          TemperatureUnits tUnits, bool fuelMapRefresh, QObject *parent = 0);
    ~CUXInterface();

    void setSerialDevice(QString device)                       { m_deviceName = device; }
    void setLambdaTrimType(c14cux_lambda_trim_type type)       { m_lambdaTrimType = type; }
    void setMAFReadingType(c14cux_airflow_type type)           { m_airflowType = type; }
    void setThrottleReadingType(c14cux_throttle_pos_type type) { m_throttlePosType = type; }

    void setEnabledSamples(QHash<SampleType,bool> samples);
    void setReadIntervals(QHash<SampleType,unsigned int> intervals);

    QString getSerialDevice() { return m_deviceName; }
    int getIntervalMsecs();

    bool isConnected();
    void disconnectFromECU();

    c14cux_feedback_mode getFeedbackMode() { return m_feedbackMode; }

    int getRoadSpeed()                { return convertSpeed(m_roadSpeedMPH); }
    int getEngineSpeedRPM()           { return m_engineSpeedRPM; }
    int getTargetIdleSpeed()          { return m_targetIdleSpeed; }
    int getCoolantTemp()              { return convertTemperature(m_coolantTempF); }
    int getFuelTemp()                 { return convertTemperature(m_fuelTempF); }
    float getThrottlePos()            { return m_throttlePos; }
    c14cux_gear getGear()             { return m_gear; }
    c14cux_faultcodes getFaultCodes() { return m_faultCodes; }
    float getMainVoltage()            { return m_mainVoltage; }
    c14cux_version getVersion()       { return c14cux_getLibraryVersion(); }
    QByteArray* getFuelMap(unsigned int fuelMapId);
    void invalidateFuelMapData();
    int getFuelMapAdjustmentFactor(unsigned int fuelMapId);
    c14cux_rpmtable getRPMTable()     { return m_rpmTable; }
    int getCurrentFuelMapIndex()      { return m_currentFuelMapIndex; }
    int getFuelMapRowIndex()          { return m_currentFuelMapRowIndex; }
    int getFuelMapColumnIndex()       { return m_currentFuelMapColumnIndex; }
    float getMAFReading()             { return m_mafReading; }
    float getIdleBypassPos()          { return m_idleBypassPos; }
    bool getFuelPumpRelayState()      { return m_fuelPumpRelayOn; }
    QByteArray* getROMImage()         { return m_romImage; }
    int getLambdaTrimOdd()            { return m_lambdaTrimOdd; }
    int getLambdaTrimEven()           { return m_lambdaTrimEven; }
    float getCOTrimVoltage()          { return m_coTrimVoltage; }
    bool isMILOn()                    { return m_milOn; }
    bool getIdleMode()                { return m_idleMode; }

    void setSpeedUnits(SpeedUnits units)             { m_speedUnits = units; }
    void setTemperatureUnits(TemperatureUnits units) { m_tempUnits = units; }
    void setPeriodicFuelMapRefresh(bool on)          { m_fuelMapRefresh = on; }

    void cancelRead();

public slots:
    void onParentThreadStarted();
    void onFaultCodesRequested();
    void onFaultCodesClearRequested();
    void onFuelMapRequested(unsigned int fuelMapId);
    void onReadROMImageRequested();
    void onStartPollingRequest();
    void onShutdownThreadRequest();
    void onFuelPumpRunRequest();
    void onIdleAirControlMovementRequest(int direction, int steps);
#ifdef ENABLE_FORCE_OPEN_LOOP
    void onForceOpenLoopRequest(bool forceOpen);
#endif
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
    void faultCodesClearSuccess(c14cux_faultcodes faultCodes);
    void faultCodesClearFailure();
    void fuelMapReady(unsigned int fuelMapId);
    void fuelMapReadFailed(unsigned int fuelMapId);
    void rpmLimitReady(int rpmLimiter);
    void rpmTableReady();
    void revisionNumberReady(int tuneRevisionNum, int checksumfixer, int ident);
    void romImageReady();
    void romImageReadFailed();
    void failedToConnect(QString dev);
    void interfaceReadyForPolling();
    void notConnected();
    void forceOpenLoopState(bool forceOpen);
    void simModeWriteSuccess();
    void simModeWriteFailure();
    void fuelMapIndexHasChanged(unsigned int fuelMapId);
    void feedbackModeHasChanged(c14cux_feedback_mode newMode);

    void data_maf(float data);
    void data_throttle(float data);

private slots:
    void onTimer();

private:
    static const int s_firstOpenLoopMap = 1;
    static const int s_lastOpenLoopMap = 3;       

    QString m_deviceName;
    c14cux_info m_cuxinfo;
    bool m_stopPolling;
    bool m_shutdownThread;
    c14cux_faultcodes m_faultCodes;
    bool m_readCanceled;
    QHash<SampleType,bool> m_enabledSamples;
    QHash<SampleType,qint64> m_lastReadTime;
    QHash<SampleType,unsigned int> m_readIntervals;

    c14cux_lambda_trim_type m_lambdaTrimType;
    c14cux_feedback_mode m_feedbackMode;
    c14cux_airflow_type m_airflowType;
    c14cux_throttle_pos_type m_throttlePosType;

    uint8_t m_roadSpeedMPH;
    uint16_t m_engineSpeedRPM;
    uint16_t m_targetIdleSpeed;
    int16_t m_coolantTempF;
    int16_t m_fuelTempF;
    float m_throttlePos;
    c14cux_gear m_gear;
    float m_mainVoltage;
    bool m_fuelMapIndexRead;
    uint8_t m_currentFuelMapIndex;
    uint8_t m_currentFuelMapRowIndex;
    uint8_t m_currentFuelMapColumnIndex;
    float m_mafReading;
    float m_idleBypassPos;
    bool m_fuelPumpRelayOn;
    int16_t m_lambdaTrimOdd;
    int16_t m_lambdaTrimEven;
    float m_coTrimVoltage;
    bool m_milOn;
    uint16_t m_rpmLimit;
    bool m_idleMode;    

    QByteArray *m_romImage;

    QByteArray m_fuelMaps[fuelMapCount];
    bool m_fuelMapDataIsCurrent[fuelMapCount];
    uint16_t m_fuelMapAdjFactors[fuelMapCount];
    c14cux_rpmtable m_rpmTable;

    SpeedUnits m_speedUnits;
    TemperatureUnits m_tempUnits;
    bool m_fuelMapRefresh;

    bool m_initComplete;

    void pollEcu();
    void clearFlagsAndData();
    ReadResult readData();
    bool readFuelMap(unsigned int fuelMapId);
    bool connectToECU();
    int convertSpeed(int speedMph);
    int convertTemperature(int tempF);
    ReadResult mergeResult(ReadResult total, ReadResult single);
    ReadResult mergeResult(ReadResult total, bool single);
    bool isDueForMeasurement(SampleType type);
    bool isSampleAppropriateForMode(SampleType type);
};

#endif // CUXINTERFACE_H

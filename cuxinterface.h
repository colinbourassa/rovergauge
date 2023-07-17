#ifndef CUXINTERFACE_H
#define CUXINTERFACE_H

#include <QMutex>
#include <utility>
#include <QObject>
#include <QString>
#include <QQueue>
#include <QHash>
#include <QByteArray>
#include <QMap>
#include "comm14cux.h"
#include "commonunits.h"
#include "simulatedecudata.h"

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
  explicit CUXInterface(QString device, unsigned int baud, SpeedUnits sUnits,
                        TemperatureUnits tUnits, bool fuelMapRefresh, bool simulateConnection,
                        QObject* parent = 0);
  ~CUXInterface();

  void setSerialDevice(QString device)
  {
    m_deviceName = device;
  }

  void setBaudRate(unsigned int baud)
  {
    m_baudRate = baud;
  }

  void setLambdaTrimType(c14cux_lambda_trim_type type)
  {
    m_lambdaTrimType = type;
  }

  void setMAFReadingType(c14cux_airflow_type type)
  {
    m_airflowType = type;
  }

  void setThrottleReadingType(c14cux_throttle_pos_type type)
  {
    m_throttlePosType = type;
  }

  void setEnabledSamples(QMap<SampleType, bool> samples);
  void setReadIntervals(QHash<SampleType, unsigned int> intervals);
  void enqueueRequest(QueueableRequest req);
  void enqueueRequest(QueueableRequest req, int data);

  QString getSerialDevice() const
  {
    return m_deviceName;
  }

  bool isConnected();
  void disconnectFromECU();

  c14cux_feedback_mode getFeedbackMode() const
  {
    return m_feedbackMode;
  }

  unsigned int getRoadSpeed() const
  {
    return (m_speedUnits == MPH) ? m_roadSpeedMPH : convertSpeed(m_roadSpeedMPH);
  }

  int getEngineSpeedRPM() const
  {
    return m_engineSpeedRPM;
  }

  int getTargetIdleSpeed() const
  {
    return m_targetIdleSpeed;
  }

  int getCoolantTemp() const
  {
    return convertTemperature(m_coolantTempF);
  }

  int getFuelTemp() const
  {
    return convertTemperature(m_fuelTempF);
  }

  float getThrottlePos() const
  {
    return m_throttlePos;
  }

  c14cux_gear getGear() const
  {
    return m_gear;
  }

  c14cux_faultcodes getFaultCodes() const
  {
    return m_faultCodes;
  }

  const QByteArray& getBatteryBackedMem() const
  {
    return m_batteryBackedMem;
  }

  float getMainVoltage() const
  {
    return m_mainVoltage;
  }

  c14cux_version getVersion() const
  {
    return c14cux_getLibraryVersion();
  }

  const QByteArray* const getFuelMap(unsigned int fuelMapId) const;
  void invalidateFuelMapData();
  int getFuelMapAdjustmentFactor(unsigned int fuelMapId) const;

  c14cux_rpmtable getRPMTable() const
  {
    return m_rpmTable;
  }

  int getCurrentFuelMapIndex() const
  {
    return m_currentFuelMapIndex;
  }

  int getFuelMapRowIndex() const
  {
    return m_currentFuelMapRowIndex;
  }

  int getFuelMapRowWeighting() const
  {
    return m_fuelMapRowWeighting;
  }

  int getFuelMapColumnIndex() const
  {
    return m_currentFuelMapColumnIndex;
  }

  int getFuelMapColWeighting() const
  {
    return m_fuelMapColWeighting;
  }

  float getMAFReading() const
  {
    return m_mafReading;
  }

  float getIdleBypassPos() const
  {
    return m_idleBypassPos;
  }

  bool getFuelPumpRelayState() const
  {
    return m_fuelPumpRelayOn;
  }

  const QByteArray& getROMImage() const
  {
    return m_romImage;
  }

  int getLambdaTrimOdd() const
  {
    return m_lambdaTrimOdd;
  }

  int getLambdaTrimEven() const
  {
    return m_lambdaTrimEven;
  }

  float getCOTrimVoltage() const
  {
    return m_coTrimVoltage;
  }

  bool isMILOn() const
  {
    return m_milOn;
  }

  bool getIdleMode() const
  {
    return m_idleMode;
  }

  float getInjectorPulseWidthMs() const
  {
    return m_injectorPulseWidthMs;
  }

  uint16_t getTune() const
  {
    return m_tune;
  }

  uint8_t getChecksumFixer() const
  {
    return m_checksumFixer;
  }

  uint16_t getIdent() const
  {
    return m_ident;
  }

  uint8_t getRowScaler(unsigned int fuelMapId) const
  {
    return m_rowScaler[fuelMapId];
  }

  uint16_t getMAFRowScaler() const
  {
    return m_mafScaler;
  }

  void setSpeedUnits(SpeedUnits units)
  {
    m_speedUnits = units;
  }

  void setTemperatureUnits(TemperatureUnits units)
  {
    m_tempUnits = units;
  }

  void setPeriodicFuelMapRefresh(bool on)
  {
    m_fuelMapRefresh = on;
  }

  void cancelRead();

  static unsigned int getBaudRate(bool doubled)
  {
    return doubled ? (C14CUX_BAUD * 2) : C14CUX_BAUD;
  }

public slots:
  void onParentThreadStarted();
  void onStartPollingRequest();
  void onShutdownThreadRequest();

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
  void batteryBackedMemReady();
  void batteryBackedMemReadFailed();
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
  void fuelMapIndexHasChanged(unsigned int fuelMapId);
  void feedbackModeHasChanged(c14cux_feedback_mode newMode);

private:
  static const int s_firstOpenLoopMap = 1;
  static const int s_lastOpenLoopMap = 3;

  const bool m_sim;
  bool m_simConnected;
  SimulatedECUData* m_simEcu;
  QMutex m_queueMutex;
  QQueue<std::pair<QueueableRequest, int> > m_reqQueue;

  QString m_deviceName;
  unsigned int m_baudRate;
  c14cux_info m_cuxinfo;
  bool m_stopPolling;
  bool m_shutdownThread;
  c14cux_faultcodes m_faultCodes;
  QByteArray m_batteryBackedMem;
  bool m_readCanceled;
  QHash<SampleType, bool> m_enabledSamples;
  QHash<SampleType, qint64> m_lastReadTime;
  QHash<SampleType, unsigned int> m_readIntervals;

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
  uint8_t m_fuelMapRowWeighting;
  uint8_t m_currentFuelMapColumnIndex;
  uint8_t m_fuelMapColWeighting;
  float m_mafReading;
  float m_idleBypassPos;
  bool m_fuelPumpRelayOn;
  int16_t m_lambdaTrimOdd;
  int16_t m_lambdaTrimEven;
  float m_coTrimVoltage;
  bool m_milOn;
  uint16_t m_rpmLimit;
  bool m_idleMode;
  uint16_t m_injectorPulseWidthUs;
  float m_injectorPulseWidthMs;
  uint16_t m_tune;
  uint8_t m_checksumFixer;
  uint16_t m_ident;
  uint8_t m_rowScaler[fuelMapCount];
  uint16_t m_mafScaler;

  QByteArray m_romImage;

  QByteArray m_fuelMaps[fuelMapCount];
  bool m_fuelMapDataIsCurrent[fuelMapCount];
  uint16_t m_fuelMapAdjFactors[fuelMapCount];
  c14cux_rpmtable m_rpmTable;

  SpeedUnits m_speedUnits;
  TemperatureUnits m_tempUnits;
  bool m_fuelMapRefresh;

  bool m_initComplete;
  bool m_rpmLimitRead;

  void zeroDisabledSamples();
  void runServiceLoop();
  void clearFlagsAndData();
  ReadResult readData();
  ReadResult readSimData();
  bool connectToECU();
  unsigned int convertSpeed(unsigned int speedMph) const;
  int convertTemperature(int tempF) const;
  static ReadResult mergeResult(ReadResult total, ReadResult single);
  static ReadResult mergeResult(ReadResult total, bool single);
  bool isDueForMeasurement(SampleType type);
  bool isSampleAppropriateForMode(SampleType type) const;
  void processQueuedRequest();
  void readFaultCodes();
  void clearFaultCodes();
  bool readFuelMap(unsigned int fuelMapId);
  void readROMImage();
  void runFuelPump();
  void driveIACMotor(int steps);
  void readRPMTable();
  void readTuneRevID();
  void readBatteryBackedMem();
};

#endif // CUXINTERFACE_H


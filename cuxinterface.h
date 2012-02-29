#ifndef CUXINTERFACE_H
#define CUXINTERFACE_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QHash>
#include <QByteArray>
#include "comm14cux.h"

class CUXInterface : public QObject
{
    Q_OBJECT
public:
    explicit CUXInterface(QString device, int interval, QObject *parent = 0);
    ~CUXInterface();

    void setSerialDevice(QString device);
    void setIntervalMsecs(int msecs);

    QString getSerialDevice();
    int getIntervalMsecs();

    bool isConnected();
    void disconnectFromECU();

    int getRoadSpeedMPH();
    int getEngineSpeedRPM();
    int getCoolantTempF();
    int getFuelTempF();
    float getThrottlePos();
    Comm14CUXGear getGear();
    Comm14CUXFaultCodes getFaultCodes();
    float getMainVoltage();
    Comm14CUXVersion getVersion();
    QByteArray* getFuelMap(int fuelMapId);
    int getCurrentFuelMapIndex();
    int getFuelMapRowIndex();
    int getFuelMapColumnIndex();
    int getMAFReading();
    QByteArray* getPROMImage();

public slots:
    void onParentThreadStarted();
    void onParentThreadFinished();
    void onFaultCodesRequested();
    void onFuelMapRequested(int fuelMapId);
    void onReadPROMImageRequested();
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
    void fuelMapReady(int fuelMapId);
    void fuelMapReadFailed(int fuelMapId);
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

    uint16_t roadSpeedMPH;
    uint16_t engineSpeedRPM;
    int16_t coolantTempF;
    int16_t fuelTempF;
    float throttlePos;
    Comm14CUXGear gear;
    float mainVoltage;
    uint8_t currentFuelMapIndex;
    uint8_t currentFuelMapRowIndex;
    uint8_t currentFuelMapColumnIndex;
    uint16_t mafReading;

    QByteArray *promImage;
    QHash<int, QByteArray*> fuelMaps;

    void pollEcu();
    bool connectToECU();
};

#endif // CUXINTERFACE_H


#include <QThread>
#include <QDateTime>
#include <string.h>
#include "cuxinterface.h"

/**
 * Constructor. Sets the serial device and poll interval in milliseconds.
 * @param device Name of (or path to) the serial device used to comminucate
 *  with the 14CUX.
 * @param interval Interval in milliseconds at which to poll the 14CUX.
 */
CUXInterface::CUXInterface(QString device, int interval, SpeedUnits sUnits,
                           TemperatureUnits tUnits, QObject *parent) :
    QObject(parent),
    intervalMsecs(interval),
    deviceName(device),
    cux(0),
    timer(0),
    stopPolling(false),
    shutdownThread(false),
    roadSpeedMPH(0),
    engineSpeedRPM(0),
    coolantTempF(0),
    fuelTempF(0),
    throttlePos(0.0),
    gear(Comm14CUXGear_NoReading),
    mainVoltage(0.0),
    currentFuelMapIndex(0),
    currentFuelMapRowIndex(0),
    currentFuelMapColumnIndex(0),
    mafReading(0.0),
    idleBypassPos(0.0),
    fuelPumpRelayOn(false),
    promImage(0),
    fuelMapAdjFactor(0),
    speedUnits(sUnits),
    tempUnits(tUnits)
{
}

/**
 * Destructor.
 */
CUXInterface::~CUXInterface()
{
}

/**
 * Returns the version of the comm14cux library being used.
 * @return Structure containing the version of the comm14cux library.
 */
Comm14CUXVersion CUXInterface::getVersion()
{
    return cux->getVersion();
}

/**
 * Reads fault codes from the 14CUX and stores in a member structure.
 */
void CUXInterface::onFaultCodesRequested()
{
    if (cux != 0)
    {
        memset(&faultCodes, 0, sizeof(faultCodes));

        if (cux->connect(deviceName.toStdString()) &&
            cux->getFaultCodes(faultCodes))
        {
            emit faultCodesReady();
        }
        else
        {
            emit faultCodesReadFailed();
        }
    }
    else
    {
        emit notConnected();
    }
}

/**
 * Clears the block of fault codes.
 */
void CUXInterface::onFaultCodesClearRequested()
{
    if (cux != 0 &&
        cux->connect(deviceName.toStdString()) &&
        cux->clearFaultCodes() &&
        cux->getFaultCodes(faultCodes))
    {
        emit faultCodesClearSuccess(faultCodes);
    }
    else
    {
        emit faultCodesClearFailure();
    }
}

/**
 * Reads the entire 16KB PROM.
 */
void CUXInterface::onReadPROMImageRequested()
{
    if (cux != 0)
    {
        if (promImage == 0)
        {
            promImage = new QByteArray(16384, 0x00);
        }

        if (cux->connect(deviceName.toStdString()) &&
            cux->dumpROM((uint8_t*)promImage->data()))
        {
            if (!readCanceled)
            {
                emit promImageReady();
            }
        }
        else
        {
            if (!readCanceled)
            {
                emit promImageReadFailed();
            }
        }

        readCanceled = false;
    }
    else
    {
        emit notConnected();
    }
}

/**
 * Respond to a signal requesting fuel map data by reading the desired fuel
 * map from the ECU, and emitting a signal when done.
 * @param fuelMapId ID of the fuel map that should be retrieved (1 through 5)
 */
void CUXInterface::onFuelMapRequested(int fuelMapId)
{
    if ((cux != 0) && cux->connect(deviceName.toStdString()))
    {
        // create a storage area for the fuel map data if it
        // doesn't already exist
        if (!fuelMaps.contains(fuelMapId))
        {
            fuelMaps.insert(fuelMapId, new QByteArray(128, 0x00));
        }

        uint8_t *buffer = (uint8_t*)(fuelMaps[fuelMapId]->data());

        if (cux->getFuelMap((int8_t)fuelMapId, fuelMapAdjFactor, buffer))
        {
            emit fuelMapReady(fuelMapId);
        }
    }
}

/**
 * Responds to a signal requesting that the fuel pump be run.
 */
void CUXInterface::onFuelPumpRunRequest()
{
    if ((cux != 0) && cux->connect(deviceName.toStdString()))
    {
        cux->runFuelPump();
    }
}

/**
 * Responds to a signal requesting that the idle air control valve be moved.
 * @param direction Direction of travel for the idle air control valve;
 *  0 to open and 1 to close
 * @param steps Number of steps to move the valve in the specified direction
 */
void CUXInterface::onIdleAirControlMovementRequest(int direction, int steps)
{
    if ((cux != 0) && cux->connect(deviceName.toStdString()))
    {
        cux->driveIdleAirControlMotor((uint8_t)direction, (uint8_t)steps);
    }
}

/**
 * Attempts to open the serial device that is connected to the 14CUX.
 * @return True if serial device was opened successfully; false otherwise.
 */
bool CUXInterface::connectToECU()
{
    bool status = false;

    // the library object should have previously been instantiated
    if (cux != 0)
    {
        status = cux->connect(deviceName.toStdString());

        if (status)
        {
            emit connected();
        }
    }

    return status;
}

/**
 * Stops polling and disconnects from the serial device.
 */
void CUXInterface::disconnectFromECU()
{
    stopPolling = true;

    if (promImage != 0)
    {
        delete promImage;
        promImage = 0;
    }

    fuelMaps.clear();
}

/**
 * Cleans up and exits the worker thread.
 */
void CUXInterface::onShutdownThreadRequest()
{
    if ((cux != 0) && cux->isConnected())
    {
        cux->disconnect();
    }
    emit disconnected();
    QThread::currentThread()->quit();
}

/**
 * Indicates whether the serial device is currently open/connected.
 * @return True when the device is connected; false otherwise.
 */
bool CUXInterface::isConnected()
{
    bool devIsConnected = false;

    if (cux != 0)
    {
        devIsConnected = cux->isConnected();
    }

    return devIsConnected;
}

/**
 * Sets the name/path of the serial device that this instance will use when
 * it connects.
 * @param device The name (e.g. "COM1") or path (e.g. "/dev/ttyUSB0") to the
 *  serial device.
 */
void CUXInterface::setSerialDevice(QString device)
{
    deviceName = device;
}

/**
 * Sets the interval between querying the ECU for new data.
 * @param msecs Poll interval in milliseconds.
 */
void CUXInterface::setIntervalMsecs(int msecs)
{
    intervalMsecs = msecs;
}

/**
 * Returns the name of the serial device that is being used to communicate
 * with the 14CUX ECU.
 * @return Serial device, such as "/dev/ttyUSB0" or "COM2"
 */
QString CUXInterface::getSerialDevice()
{
    return deviceName;
}

/**
 * Returns the polling interval in milliseconds.
 * @return Milliseconds between each attempt to poll the ECU for data.
 */
int CUXInterface::getIntervalMsecs()
{
    return intervalMsecs;
}

/**
 * Cleans up dynamically-allocated objects when the thread finishes.
 */
void CUXInterface::onParentThreadFinished()
{
    if (cux != 0)
    {
        delete cux;
        cux = 0;
    }

    if (timer != 0)
    {
        delete timer;
        timer = 0;
    }
}

/**
 * Responds to the parent thread being started by instantiating the library
 * object and a timer (if necessary), and emitting a signal indicating that
 * the interface is ready.
 */
void CUXInterface::onParentThreadStarted()
{
    if (cux == 0)
    {
        cux = new Comm14CUX();
    }

    if (timer == 0)
    {
        timer = new QTimer(this);
        timer->setSingleShot(true);
        connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    }

    emit interfaceReadyForPolling();
}

/**
 * Responds to a signal to start polling the ECU.
 */
void CUXInterface::onStartPollingRequest()
{
    if (connectToECU())
    {
        stopPolling = false;
        shutdownThread = false;
        pollEcu();
    }
    else
    {
        emit failedToConnect(deviceName);
    }
}

/**
 * Reads specific locations from the ECU and stores the data locally, then
 * calculates the amount of time to wait until the next read cycle. If the
 * timer interval is set to less than the amount of time that it takes to
 * actually read and process the data, then the timer interval will be set
 * to a minimum value of 10ms.
 */
void CUXInterface::pollEcu()
{
    qint64 nextDue = QDateTime::currentMSecsSinceEpoch() + intervalMsecs;

    // if we're being asked to stop the thread, or if the 14CUX interface is
    // no longer connected
    if (stopPolling || shutdownThread ||
        (cux == 0) || (!cux->isConnected()) )
    {
        if ((cux != 0) && cux->isConnected())
        {
            cux->disconnect();
        }
        emit disconnected();

        if (shutdownThread)
        {
            QThread::currentThread()->quit();
        }
    }
    else
    {
        bool success = false;

        // closely-grouped 16-bit values read consecutively for read efficiency...
        success |= cux->getMainVoltage(mainVoltage);
        success |= cux->getMAFReading(mafReading);
        success |= cux->getThrottlePosition(throttlePos);
        success |= cux->getEngineRPM(engineSpeedRPM);

        // ...and likewise with the 8-bit values low in memory...
        success |= cux->getFuelMapRowIndex(currentFuelMapRowIndex);
        success |= cux->getFuelMapColumnIndex(currentFuelMapColumnIndex);
        success |= cux->getCoolantTemp(coolantTempF);
        success |= cux->getIdleBypassMotorPosition(idleBypassPos);

        // ...and higher in memory
        success |= cux->getGearSelection(gear);
        success |= cux->getRoadSpeed(roadSpeedMPH);
        success |= cux->getFuelTemp(fuelTempF);
        success |= cux->getCurrentFuelMap(currentFuelMapIndex);

        // this one's done separately, since it's way at the bottom of memory
        success |= cux->getFuelPumpRelayState(fuelPumpRelayOn);

        if (success)
        {
            emit readSuccess();
            emit dataReady();
        }
        else
        {
            emit readError();
        }

        int msecUntilNext = nextDue - QDateTime::currentMSecsSinceEpoch();
        if (msecUntilNext < 0)
        {
            msecUntilNext = 0;
        }
        timer->start(msecUntilNext);
    }
}

/**
 * Responds to the single-shot timer expiring by polling the ECU for new data.
 */
void CUXInterface::onTimer()
{
    pollEcu();
}

/**
 * Cancels the pending read operation.
 */
void CUXInterface::cancelRead()
{
    readCanceled = true;
    cux->cancelRead();
}

/**
 * Returns the last-read road speed value.
 * @return Last-read road speed in the desired units.
 */
int CUXInterface::getRoadSpeed()
{
    return convertSpeed(roadSpeedMPH);
}

/**
 * Returns the last-read engine speed value.
 * @return Last-read engine speed in RPM
 */
int CUXInterface::getEngineSpeedRPM()
{
    return engineSpeedRPM;
}

/**
 * Returns the last-read coolant temperature value.
 * @return Last-read coolant temperature in the desired units
 */
int CUXInterface::getCoolantTemp()
{
    return convertTemperature(coolantTempF);
}

/**
 * Returns the last-read fuel temperature value.
 * @return Last-read fuel temperatures in the desired units
 */
int CUXInterface::getFuelTemp()
{
    return convertTemperature(fuelTempF);
}

/**
 * Returns the last-read throttle position value.
 * @return Last-read throttle position
 */
float CUXInterface::getThrottlePos()
{
    return throttlePos;
}

/**
 * Returns the last-read neutral switch value.
 * @return Last-read neutral switch state
 */
Comm14CUXGear CUXInterface::getGear()
{
    return gear;
}

/**
 * Returns the last-read main voltage value.
 * @return Last-read main voltage
 */
float CUXInterface::getMainVoltage()
{
    return mainVoltage;
}

/**
 * Returns the last-read fault code structure.
 * @return Last-read fault codes.
 */
Comm14CUXFaultCodes CUXInterface::getFaultCodes()
{
    return faultCodes;
}

/**
 * Returns the data for a particular fuel map.
 * @param fuelMapId ID of the fuel map to retrieve
 * @return Pointer to the container holding the fuel map data, or 0 if the
 *   fuel map in question has not yet been retrieved
 */
QByteArray* CUXInterface::getFuelMap(int fuelMapId)
{
    QByteArray* map = 0;

    if (fuelMaps.contains(fuelMapId))
    {
        map = fuelMaps[fuelMapId];
    }

    return map;
}

/**
 * Returns the current row index being used to retrieve fueling values.
 * @return Fuel map row index
 */
int CUXInterface::getFuelMapRowIndex()
{
    return currentFuelMapRowIndex;
}

/**
 * Returns the current column index being used to retrieve fueling values.
 * @return Fuel map column index
 */
int CUXInterface::getFuelMapColumnIndex()
{
    return currentFuelMapColumnIndex;
}

/**
 * Returns the index of the currently-selected fuel map.
 * @return ID of selected fuel map (1 through 5)
 */
int CUXInterface::getCurrentFuelMapIndex()
{
    return currentFuelMapIndex;
}

/**
 * Returns the last-read MAF reading.
 * @return Last-read MAF reading
 */
float CUXInterface::getMAFReading()
{
    return mafReading;
}

/**
 * Returns the last-read PROM image.
 * @return Last-read PROM image (16KB array)
 */
QByteArray* CUXInterface::getPROMImage()
{
    return promImage;
}

/**
 * Returns the last-read fuel map adjustment factor.
 * @return Last-read fuel map adjustment factor
 */
int CUXInterface::getFuelMapAdjustmentFactor()
{
    return fuelMapAdjFactor;
}

/**
 * Returns the last-read idle bypass motor position.
 * @return Last-read idle bypass motor position
 */
float CUXInterface::getIdleBypassPos()
{
    return idleBypassPos;
}

/**
 * Returns the last-read fuel pump relay state.
 * @return Last-read fuel pump relay state
 */
bool CUXInterface::getFuelPumpRelayState()
{
    return fuelPumpRelayOn;
}

/**
 * Sets the desired output units for temperature measurements
 * @param units Desired units
 */
void CUXInterface::setSpeedUnits(SpeedUnits units)
{
    speedUnits = units;
}

/**
 * Sets the desired output units for temperature measurements
 * @param units Desired units
 */
void CUXInterface::setTemperatureUnits(TemperatureUnits units)
{
    tempUnits = units;
}

/**
 * Converts speed in miles per hour to the desired units.
 * @param speedMph Speed in miles per hour
 * @return Speed in the desired units
 */
int CUXInterface::convertSpeed(int speedMph)
{
    double speed = speedMph;

    switch (speedUnits)
    {
    case FPS:
        speed *= 1.46666667;
        break;
    case KPH:
        speed *= 1.609344;
        break;
    default:
        break;
    }

    return (int)speed;
}

/**
 * Converts temperature in Fahrenheit degrees to the desired units.
 * @param tempF Temperature in Fahrenheit degrees
 * @return Temperature in the desired units
 */
int CUXInterface::convertTemperature(int tempF)
{
    double temp = tempF;

    switch (tempUnits)
    {
    case Celcius:
        temp = (temp - 32) * (0.5555556);
        break;
    case Fahrenheit:
    default:
        break;
    }

    return (int)temp;
}

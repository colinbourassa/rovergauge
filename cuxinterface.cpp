#include <QThread>
#include <QDateTime>
#include <string.h>
#include "cuxinterface.h"

/**
 * Constructor. Sets the serial device and poll interval in milliseconds.
 * @param device Name of (or path to) the serial device used to comminucate
 *  with the 14CUX.
 * @param sUnits Units to be used when expressing road speed
 * @param tUnits Units to be used when expressing coolant/fuel temperature
 */
CUXInterface::CUXInterface(QString device, SpeedUnits sUnits, TemperatureUnits tUnits,
                           QObject *parent) :
    QObject(parent),
    m_deviceName(device),
    m_cux(0),
    m_timer(0),
    m_stopPolling(false),
    m_shutdownThread(false),
    m_readCanceled(false),
    m_readCount(0),
    m_lambdaTrimType(1),
    m_airflowType(Comm14CUXAirflowType_Linearized),
    m_throttlePosType(Comm14CUXThrottlePosType_Corrected),
    m_roadSpeedMPH(0),
    m_engineSpeedRPM(0),
    m_targetIdleSpeed(0),
    m_coolantTempF(0),
    m_fuelTempF(0),
    m_throttlePos(0.0),
    m_gear(Comm14CUXGear_NoReading),
    m_mainVoltage(0.0),
    m_currentFuelMapIndex(0),
    m_currentFuelMapRowIndex(0),
    m_currentFuelMapColumnIndex(0),
    m_mafReading(0.0),
    m_idleBypassPos(0.0),
    m_fuelPumpRelayOn(false),
    m_leftLambdaTrim(0),
    m_rightLambdaTrim(0),
    m_milOn(false),
    m_idleMode(false),
    m_promImage(0),
    m_fuelMapAdjFactor(0),
    m_speedUnits(sUnits),
    m_tempUnits(tUnits),
    m_lastMidFreqReadTime(0),
    m_lastLowFreqReadTime(0)
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
    return Comm14CUX::getVersion();
}

/**
 * Reads fault codes from the 14CUX and stores in a member structure.
 */
void CUXInterface::onFaultCodesRequested()
{
    if (m_cux != 0)
    {
        memset(&m_faultCodes, 0, sizeof(m_faultCodes));

        if (m_cux->connect(m_deviceName.toStdString().c_str()) &&
            m_cux->getFaultCodes(m_faultCodes))
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
    if (m_cux != 0 &&
        m_cux->connect(m_deviceName.toStdString().c_str()) &&
        m_cux->clearFaultCodes() &&
        m_cux->getFaultCodes(m_faultCodes))
    {
        emit faultCodesClearSuccess(m_faultCodes);
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
    if (m_cux != 0)
    {
        if (m_promImage == 0)
        {
            m_promImage = new QByteArray(16384, 0x00);
        }

        if (m_cux->connect(m_deviceName.toStdString().c_str()) &&
            m_cux->dumpROM((uint8_t*)m_promImage->data()))
        {
            if (!m_readCanceled)
            {
                emit promImageReady();
            }
        }
        else
        {
            if (!m_readCanceled)
            {
                emit promImageReadFailed();
            }
        }

        m_readCanceled = false;
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
    if ((m_cux != 0) && m_cux->connect(m_deviceName.toStdString().c_str()))
    {
        // create a storage area for the fuel map data if it
        // doesn't already exist
        if (!m_fuelMaps.contains(fuelMapId))
        {
            m_fuelMaps.insert(fuelMapId, new QByteArray(128, 0x00));
        }

        uint8_t *buffer = (uint8_t*)(m_fuelMaps[fuelMapId]->data());

        if (m_cux->getFuelMap((int8_t)fuelMapId, m_fuelMapAdjFactor, buffer))
        {
            emit fuelMapReady(fuelMapId);
        }

        if (m_cux->getRPMLimit(m_rpmLimit))
        {
            emit rpmLimitReady(m_rpmLimit);
        }
    }
}

/**
 * Responds to a signal requesting that the fuel pump be run.
 */
void CUXInterface::onFuelPumpRunRequest()
{
    if ((m_cux != 0) && m_cux->connect(m_deviceName.toStdString().c_str()))
    {
        m_cux->runFuelPump();
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
    if ((m_cux != 0) && m_cux->connect(m_deviceName.toStdString().c_str()))
    {
        m_cux->driveIdleAirControlMotor((uint8_t)direction, (uint8_t)steps);
    }
    else
    {
        emit notConnected();
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
    if (m_cux != 0)
    {
        status = m_cux->connect(m_deviceName.toStdString().c_str());

        if (status)
        {
            emit connected();

            if (m_cux->getTuneRevision(m_tuneRevision))
            {
                emit revisionNumberReady(m_tuneRevision);
            }
        }
    }

    return status;
}

/**
 * Stops polling and disconnects from the serial device.
 */
void CUXInterface::disconnectFromECU()
{
    m_stopPolling = true;

    if (m_promImage != 0)
    {
        delete m_promImage;
        m_promImage = 0;
    }

    m_fuelMaps.clear();
}

/**
 * Cleans up and exits the worker thread.
 */
void CUXInterface::onShutdownThreadRequest()
{
    if ((m_cux != 0) && m_cux->isConnected())
    {
        m_cux->disconnect();
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

    if (m_cux != 0)
    {
        devIsConnected = m_cux->isConnected();
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
    m_deviceName = device;
}

/**
 * Returns the name of the serial device that is being used to communicate
 * with the 14CUX ECU.
 * @return Serial device, such as "/dev/ttyUSB0" or "COM2"
 */
QString CUXInterface::getSerialDevice()
{
    return m_deviceName;
}

/**
 * Cleans up dynamically-allocated objects when the thread finishes.
 */
void CUXInterface::onParentThreadFinished()
{
    if (m_cux != 0)
    {
        delete m_cux;
        m_cux = 0;
    }

    if (m_timer != 0)
    {
        delete m_timer;
        m_timer = 0;
    }
}

/**
 * Responds to the parent thread being started by instantiating the library
 * object and a timer (if necessary), and emitting a signal indicating that
 * the interface is ready.
 */
void CUXInterface::onParentThreadStarted()
{
    if (m_cux == 0)
    {
        m_cux = new Comm14CUX();
    }

    if (m_timer == 0)
    {
        m_timer = new QTimer(this);
        m_timer->setSingleShot(true);
        connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
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
        m_stopPolling = false;
        m_shutdownThread = false;
        pollEcu();
    }
    else
    {
        emit failedToConnect(m_deviceName);
    }
}

/**
 * Reads specific locations from the ECU and stores the data locally.
 */
void CUXInterface::pollEcu()
{
    ReadResult res = ReadResult_NoStatement;

    // if we're being asked to stop the thread, or if the 14CUX interface is
    // no longer connected...
    if (m_stopPolling || m_shutdownThread ||
        (m_cux == 0) || (!m_cux->isConnected()) )
    {
        if ((m_cux != 0) && m_cux->isConnected())
        {
            m_cux->disconnect();
        }
        emit disconnected();

        if (m_shutdownThread)
        {
            QThread::currentThread()->quit();
        }
    }
    else
    {
        res = readData();
        if (res == ReadResult_Success)
        {
            emit readSuccess();
            emit dataReady();
        }
        else if (res == ReadResult_Failure)
        {
            emit readError();
        }

        m_readCount++;
        m_timer->start(0);
    }
}

/**
 * Reads data from the 14CUX via calls to the library, and stores the data in
 * member variables.
 * @return True if at least one value was read successfully; false otherwise.
 */
CUXInterface::ReadResult CUXInterface::readData()
{
    ReadResult totalResult = ReadResult_NoStatement;
    qint64 now = QDateTime::currentMSecsSinceEpoch();

    totalResult = mergeResult(totalResult, readHighFreqData());

    if (now > (m_lastMidFreqReadTime + 200))
    {
        totalResult = mergeResult(totalResult, readMidFreqData());
    }
    if (now > (m_lastLowFreqReadTime + 800))
    {
        totalResult = mergeResult(totalResult, readLowFreqData());
    }

    return totalResult;
}

CUXInterface::ReadResult CUXInterface::readHighFreqData()
{
    ReadResult result = ReadResult_NoStatement;

    if (m_enabledSamples[SampleType_MAF])
        result = mergeResult(result, m_cux->getMAFReading(m_airflowType, m_mafReading));

    if (m_enabledSamples[SampleType_Throttle])
        result = mergeResult(result, m_cux->getThrottlePosition(m_throttlePosType, m_throttlePos));

    // if the frontend if expecting short-term lambda trim
    // (as opposed to long-term trim)
    if (m_enabledSamples[SampleType_LambdaTrim] && (m_lambdaTrimType == 1))
    {
        result = mergeResult(result, m_cux->getLambdaTrimShort(Comm14CUXBank_Left, m_leftLambdaTrim));
        result = mergeResult(result, m_cux->getLambdaTrimShort(Comm14CUXBank_Right, m_rightLambdaTrim));
    }

    if (m_enabledSamples[SampleType_EngineRPM])
        result = mergeResult(result, m_cux->getEngineRPM(m_engineSpeedRPM));

    if (m_enabledSamples[SampleType_FuelMap])
    {
        result = mergeResult(result, m_cux->getFuelMapRowIndex(m_currentFuelMapRowIndex));
        result = mergeResult(result, m_cux->getFuelMapColumnIndex(m_currentFuelMapColumnIndex));
    }

    if (m_enabledSamples[SampleType_IdleBypassPosition])
        result = mergeResult(result, m_cux->getIdleBypassMotorPosition(m_idleBypassPos));

    return result;
}

CUXInterface::ReadResult CUXInterface::readMidFreqData()
{
    ReadResult result = ReadResult_NoStatement;

    // if the frontend is expecting long-term lambda trim
    // (as opposed to short-term trim)
    if (m_enabledSamples[SampleType_LambdaTrim] && (m_lambdaTrimType == 2))
    {
        result = mergeResult(result, m_cux->getLambdaTrimLong(Comm14CUXBank_Left, m_leftLambdaTrim));
        result = mergeResult(result, m_cux->getLambdaTrimLong(Comm14CUXBank_Right, m_rightLambdaTrim));
    }

    if (m_enabledSamples[SampleType_MainVoltage])
        result = mergeResult(result, m_cux->getMainVoltage(m_mainVoltage));

    if (m_enabledSamples[SampleType_TargetIdleRPM])
    {
        result = mergeResult(result, m_cux->getTargetIdle(m_targetIdleSpeed));
        result = mergeResult(result, m_cux->getIdleMode(m_idleMode));
    }

    if (m_enabledSamples[SampleType_FuelPumpRelay])
        result = mergeResult(result, m_cux->getFuelPumpRelayState(m_fuelPumpRelayOn));

    if (m_enabledSamples[SampleType_GearSelection])
        result = mergeResult(result, m_cux->getGearSelection(m_gear));

    if (m_enabledSamples[SampleType_RoadSpeed])
        result = mergeResult(result, m_cux->getRoadSpeed(m_roadSpeedMPH));

    if (result == ReadResult_Success)
    {
        m_lastMidFreqReadTime = QDateTime::currentMSecsSinceEpoch();
    }

    return result;
}

/**
 * Reads data that changes at a low rate, such as temperatures.
 * @return True if a read was scheduled and completed successfully,
 *  false otherwise.
 */
CUXInterface::ReadResult CUXInterface::readLowFreqData()
{
    ReadResult result = ReadResult_NoStatement;

    // attempt to read the MIL status; if it can't be read,
    // default it to off on the display
    if (!m_cux->isMILOn(m_milOn))
    {
        m_milOn = false;
    }

    // alternate between reading coolant temperature and fuel temperature
    if (m_enabledSamples[SampleType_EngineTemperature] && (m_readCount % 2 == 0))
        result = mergeResult(result, m_cux->getCoolantTemp(m_coolantTempF));
    else if (m_enabledSamples[SampleType_FuelTemperature])
        result = mergeResult(result, m_cux->getFuelTemp(m_fuelTempF));

    // less frequently, check the ID of the current fuel map
    // (this would only change as a result of a different
    //  tune resistor being switched in)
    if (m_enabledSamples[SampleType_FuelMap] && (m_readCount % 7 == 0))
        result = mergeResult(result, m_cux->getCurrentFuelMap(m_currentFuelMapIndex));

    if (result == ReadResult_Success)
    {
        m_lastLowFreqReadTime = QDateTime::currentMSecsSinceEpoch();
    }

    return result;
}

/**
 * Responds to the single-shot timer expiring by polling the ECU for new data.
 */
void CUXInterface::onTimer()
{
    pollEcu();
}

/**
 * Merges the result of a group of read attempts with a running aggregation of read results.
 */
CUXInterface::ReadResult CUXInterface::mergeResult(ReadResult total, ReadResult single)
{
    ReadResult returnRes = total;

    if (((total == ReadResult_NoStatement) || (single == ReadResult_Success)) ||
        ((total == ReadResult_Failure)     && (single == ReadResult_Success)))
    {
        returnRes = single;
    }

    return returnRes;
}

/**
 * Merges the result of a read attempt with a running aggregation of read results.
 */
CUXInterface::ReadResult CUXInterface::mergeResult(ReadResult total, bool single)
{
    ReadResult result = total;

    if (total == ReadResult_NoStatement)
    {
        result = single ? ReadResult_Success : ReadResult_Failure;
    }
    else if ((total == ReadResult_Failure) && single)
    {
        result = ReadResult_Success;
    }

    return result;
}

/**
 * Cancels the pending read operation.
 */
void CUXInterface::cancelRead()
{
    m_readCanceled = true;
    m_cux->cancelRead();
}

/**
 * Returns the last-read road speed value.
 * @return Last-read road speed in the desired units.
 */
int CUXInterface::getRoadSpeed()
{
    return convertSpeed(m_roadSpeedMPH);
}

/**
 * Returns the last-read engine speed value.
 * @return Last-read engine speed in RPM
 */
int CUXInterface::getEngineSpeedRPM()
{
    return m_engineSpeedRPM;
}

/**
 * Returns the last-read target idle speed value.
 * @return Last-read target idle speed in RPM
 */
int CUXInterface::getTargetIdleSpeed()
{
    return m_targetIdleSpeed;
}

/**
 * Returns the last-read coolant temperature value.
 * @return Last-read coolant temperature in the desired units
 */
int CUXInterface::getCoolantTemp()
{
    return convertTemperature(m_coolantTempF);
}

/**
 * Returns the last-read fuel temperature value.
 * @return Last-read fuel temperatures in the desired units
 */
int CUXInterface::getFuelTemp()
{
    return convertTemperature(m_fuelTempF);
}

/**
 * Returns the last-read throttle position value.
 * @return Last-read throttle position
 */
float CUXInterface::getThrottlePos()
{
    return m_throttlePos;
}

/**
 * Returns the last-read neutral switch value.
 * @return Last-read neutral switch state
 */
Comm14CUXGear CUXInterface::getGear()
{
    return m_gear;
}

/**
 * Returns the last-read main voltage value.
 * @return Last-read main voltage
 */
float CUXInterface::getMainVoltage()
{
    return m_mainVoltage;
}

/**
 * Returns the last-read fault code structure.
 * @return Last-read fault codes.
 */
Comm14CUXFaultCodes CUXInterface::getFaultCodes()
{
    return m_faultCodes;
}

bool CUXInterface::isMILOn()
{
    return m_milOn;
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

    if (m_fuelMaps.contains(fuelMapId))
    {
        map = m_fuelMaps[fuelMapId];
    }

    return map;
}

/**
 * Returns the current row index being used to retrieve fueling values.
 * @return Fuel map row index
 */
int CUXInterface::getFuelMapRowIndex()
{
    return m_currentFuelMapRowIndex;
}

/**
 * Returns the current column index being used to retrieve fueling values.
 * @return Fuel map column index
 */
int CUXInterface::getFuelMapColumnIndex()
{
    return m_currentFuelMapColumnIndex;
}

/**
 * Returns the index of the currently-selected fuel map.
 * @return ID of selected fuel map (1 through 5)
 */
int CUXInterface::getCurrentFuelMapIndex()
{
    return m_currentFuelMapIndex;
}

/**
 * Returns the last-read MAF reading.
 * @return Last-read MAF reading
 */
float CUXInterface::getMAFReading()
{
    return m_mafReading;
}

/**
 * Returns the last-read PROM image.
 * @return Last-read PROM image (16KB array)
 */
QByteArray* CUXInterface::getPROMImage()
{
    return m_promImage;
}

/**
 * Returns the last-read fuel map adjustment factor.
 * @return Last-read fuel map adjustment factor
 */
int CUXInterface::getFuelMapAdjustmentFactor()
{
    return m_fuelMapAdjFactor;
}

/**
 * Returns the last-read idle bypass motor position.
 * @return Last-read idle bypass motor position
 */
float CUXInterface::getIdleBypassPos()
{
    return m_idleBypassPos;
}

/**
 * Returns the last-read fuel pump relay state.
 * @return Last-read fuel pump relay state
 */
bool CUXInterface::getFuelPumpRelayState()
{
    return m_fuelPumpRelayOn;
}

/**
 * Sets the desired output units for temperature measurements
 * @param units Desired units
 */
void CUXInterface::setSpeedUnits(SpeedUnits units)
{
    m_speedUnits = units;
}

/**
 * Sets the desired output units for temperature measurements
 * @param units Desired units
 */
void CUXInterface::setTemperatureUnits(TemperatureUnits units)
{
    m_tempUnits = units;
}

/**
 * Sets the type of lambda trim to read (short- or long-term)
 * @param isShortTerm Set to true if short-term lambda trim should be read;
 *   set to false for long-term
 */
void CUXInterface::setLambdaTrimType(int type)
{
    m_lambdaTrimType = type;
}

/**
 * Sets the type of MAF reading to take (direct or linearized).
 * @param type Selects either Direct or Linearized MAF readings.
 */
void CUXInterface::setMAFReadingType(Comm14CUXAirflowType type)
{
    m_airflowType = type;
}

/**
 * Sets the type of throttle position reading to take (corrected or absolute).
 * @param type Selects either Absolute or Corrected throttle position type
 */
void CUXInterface::setThrottleReadingType(Comm14CUXThrottlePosType type)
{
    m_throttlePosType = type;
}

/**
 * Returns the last-read lambda-based fuel trim for the left bank
 * @return Last-read lambda-based fuel trim
 */
int CUXInterface::getLeftLambdaTrim()
{
    return m_leftLambdaTrim;
}

/**
 * Returns the last-read lambda-based fuel trim for the right bank
 * @return Last-read lambda-based fuel trim
 */
int CUXInterface::getRightLambdaTrim()
{
    return m_rightLambdaTrim;
}

bool CUXInterface::getIdleMode()
{
    return m_idleMode;
}

/**
 * Converts speed in miles per hour to the desired units.
 * @param speedMph Speed in miles per hour
 * @return Speed in the desired units
 */
int CUXInterface::convertSpeed(int speedMph)
{
    double speed = speedMph;

    switch (m_speedUnits)
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

    switch (m_tempUnits)
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

/**
 * Updates the list of sample types that are enabled/disabled for reading
 */
void CUXInterface::setEnabledSamples(QHash<SampleType, bool> samples)
{
    // the fields are updated one at a time, because a replacement of the entire
    // hash table (using the assignment operator) can disrupt other threads that
    // are reading the table at that time
    foreach (SampleType field, samples.keys())
    {
        m_enabledSamples[field] = samples[field];
    }
}

void CUXInterface::onSimModeWriteRequest(bool enableSimMode, SimulationInputValues simVals)
{
    if ((m_cux != 0) &&
         m_cux->connect(m_deviceName.toStdString().c_str()))
    {
        bool success = true;

        success &= m_cux->writeMem(0x2060, simVals.inertiaSwitch);
        success &= m_cux->writeMem(0x2061, simVals.heatedScreen);
        success &= m_cux->writeMem(0x2062, (uint8_t)((simVals.maf & (0xFF00)) >> 8));
        success &= m_cux->writeMem(0x2063, (uint8_t)(simVals.maf & (0x00FF)));
        success &= m_cux->writeMem(0x2064, (uint8_t)((simVals.throttle & (0xFF00)) >> 8));
        success &= m_cux->writeMem(0x2065, (uint8_t)(simVals.throttle & (0x00FF)));
        success &= m_cux->writeMem(0x2066, simVals.coolantTemp);
        success &= m_cux->writeMem(0x2067, simVals.neutralSwitch);
        success &= m_cux->writeMem(0x2068, simVals.airConLoad);
        success &= m_cux->writeMem(0x2069, simVals.roadSpeed);
        success &= m_cux->writeMem(0x206A, simVals.mainRelay);
        success &= m_cux->writeMem(0x206B, simVals.mafTrim);
        success &= m_cux->writeMem(0x206C, simVals.tuneResistor);
        success &= m_cux->writeMem(0x206D, simVals.fuelTemp);
        success &= m_cux->writeMem(0x206E, simVals.o2LeftDutyCycle);
        success &= m_cux->writeMem(0x206F, simVals.o2SensorReference);
        success &= m_cux->writeMem(0x2070, simVals.diagnosticPlug);
        success &= m_cux->writeMem(0x2071, simVals.o2RightDutyCycle);

        if (enableSimMode && success)
        {
            success &= m_cux->writeMem(0x2072, 0x55);
            success &= m_cux->clearFaultCodes();
        }

        if (success)
        {
            emit simModeWriteSuccess();
        }
        else
        {
            emit simModeWriteFailure();
        }
    }
    else
    {
        emit notConnected();
    }
}


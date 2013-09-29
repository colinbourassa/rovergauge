#include <QThread>
#include <QDateTime>
#include <string.h>
#include "cuxinterface.h"

/**
 * Constructor. Sets the serial device and measurement units.
 * @param device Name of (or path to) the serial device used to comminucate
 *  with the 14CUX.
 * @param sUnits Units to be used when expressing road speed
 * @param tUnits Units to be used when expressing coolant/fuel temperature
 */
CUXInterface::CUXInterface(QString device, SpeedUnits sUnits, TemperatureUnits tUnits,
                           QObject *parent) :
    QObject(parent),
    m_deviceName(device),
    m_timer(0),
    m_stopPolling(false),
    m_shutdownThread(false),
    m_readCanceled(false),
    m_readCount(0),
    m_lambdaTrimType(C14CUX_LambdaTrimType_ShortTerm),
    m_airflowType(C14CUX_AirflowType_Linearized),
    m_throttlePosType(C14CUX_ThrottlePosType_Absolute),
    m_roadSpeedMPH(0),
    m_engineSpeedRPM(0),
    m_targetIdleSpeed(0),
    m_coolantTempF(0),
    m_fuelTempF(0),
    m_throttlePos(0.0),
    m_gear(C14CUX_Gear_NoReading),
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
    m_romImage(0),
    m_speedUnits(sUnits),
    m_tempUnits(tUnits),
    m_lastMidFreqReadTime(0),
    m_lastLowFreqReadTime(0),
    m_initComplete(false)
{
    for (unsigned int idx = 0; idx < fuelMapCount; ++idx)
    {
        m_fuelMaps[idx].fill(0x00, 128);
        m_fuelMapDataIsCurrent[idx] = false;
    }
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
c14cux_version CUXInterface::getVersion()
{
    return c14cux_getLibraryVersion();
}

/**
 * Reads fault codes from the 14CUX and stores in a member structure.
 */
void CUXInterface::onFaultCodesRequested()
{
    if (m_initComplete && c14cux_isConnected(&m_cuxinfo))
    {
        memset(&m_faultCodes, 0, sizeof(m_faultCodes));

        if (c14cux_getFaultCodes(&m_cuxinfo, &m_faultCodes))
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
    if (m_initComplete && c14cux_isConnected(&m_cuxinfo))
    {
        if (c14cux_clearFaultCodes(&m_cuxinfo) &&
            c14cux_getFaultCodes(&m_cuxinfo, &m_faultCodes))
        {
            emit faultCodesClearSuccess(m_faultCodes);
        }
        else
        {
            emit faultCodesClearFailure();
        }
    }
    else
    {
        emit notConnected();
    }
}

/**
 * Reads the entire 16KB ROM.
 */
void CUXInterface::onReadROMImageRequested()
{
    if (m_initComplete && c14cux_isConnected(&m_cuxinfo))
    {
        if (m_romImage == 0)
        {
            m_romImage = new QByteArray(16384, 0x00);
        }

        if (c14cux_dumpROM(&m_cuxinfo, (uint8_t*)m_romImage->data()))
        {
            if (!m_readCanceled)
            {
                emit romImageReady();
            }
        }
        else
        {
            if (!m_readCanceled)
            {
                emit romImageReadFailed();
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
void CUXInterface::onFuelMapRequested(unsigned int fuelMapId)
{
    if (m_initComplete && c14cux_isConnected(&m_cuxinfo))
    {
        uint8_t *buffer = (uint8_t*)(m_fuelMaps[fuelMapId].data());
        uint16_t adjFactor = 0;

        if (c14cux_getFuelMap(&m_cuxinfo, (int8_t)fuelMapId, &adjFactor, buffer))
        {
            m_fuelMapAdjFactors[fuelMapId] = adjFactor;
            m_fuelMapDataIsCurrent[fuelMapId] = true;
            emit fuelMapReady(fuelMapId);
        }

        if (c14cux_getRPMLimit(&m_cuxinfo, &m_rpmLimit))
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
    if (m_initComplete && c14cux_isConnected(&m_cuxinfo))
    {
        c14cux_runFuelPump(&m_cuxinfo);
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
    if (m_initComplete && c14cux_isConnected(&m_cuxinfo))
    {
        c14cux_driveIdleAirControlMotor(&m_cuxinfo, (uint8_t)direction, (uint8_t)steps);
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
    bool status = c14cux_connect(&m_cuxinfo, m_deviceName.toStdString().c_str());    

    if (status)
    {
        emit connected();

        if (c14cux_getTuneRevision(&m_cuxinfo, &m_tuneRevision))
        {
            emit revisionNumberReady(m_tuneRevision);
        }
    }

    return status;
}

/**
 * Sets a flag that will cause us to stop polling and disconnect from the serial device.
 */
void CUXInterface::disconnectFromECU()
{
    m_stopPolling = true;

    if (m_romImage != 0)
    {
        delete m_romImage;
        m_romImage = 0;
    }

    // invalidate the stored fuel map data so that it is retrieved again upon reconnecting
    for (unsigned int idx = 0; idx < fuelMapCount; ++idx)
    {
        m_fuelMapDataIsCurrent[idx] = false;
    }
}

/**
 * Cleans up and exits the worker thread.
 */
void CUXInterface::onShutdownThreadRequest()
{
    if (m_initComplete && c14cux_isConnected(&m_cuxinfo))
    {
        c14cux_disconnect(&m_cuxinfo);
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
    return (m_initComplete && c14cux_isConnected(&m_cuxinfo));
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
    // Initialize the interface state info struct here, so that
    // it's in the context of the thread that will use it.
    if (!m_initComplete)
    {
        c14cux_init(&m_cuxinfo);
        m_initComplete = true;
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
#ifdef WIN32
        QString simpleDeviceName = m_deviceName;
        if (simpleDeviceName.indexOf("\\\\.\\") == 0)
        {
            simpleDeviceName.remove(0, 4);
        }
        emit failedToConnect(simpleDeviceName);
#else
        emit failedToConnect(m_deviceName);
#endif
    }
}

/**
 * Reads specific locations from the ECU and stores the data locally.
 */
void CUXInterface::pollEcu()
{
    ReadResult res = ReadResult_NoStatement;

    bool connected = c14cux_isConnected(&m_cuxinfo);

    // if we're being asked to stop the thread, or if the 14CUX interface is
    // no longer connected...
    if (m_stopPolling || m_shutdownThread || !connected)
    {
        if (connected)
        {
            c14cux_disconnect(&m_cuxinfo);
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

/**
 * Reads data that changes at a high rate, such as MAF reading and throttle position.
 * @return True if a read was scheduled and completed successfully,
 *  false otherwise.
 */
CUXInterface::ReadResult CUXInterface::readHighFreqData()
{
    ReadResult result = ReadResult_NoStatement;

    if (m_enabledSamples[SampleType_MAF])
        result = mergeResult(result, c14cux_getMAFReading(&m_cuxinfo, m_airflowType, &m_mafReading));

    if (m_enabledSamples[SampleType_Throttle])
        result = mergeResult(result, c14cux_getThrottlePosition(&m_cuxinfo, m_throttlePosType, &m_throttlePos));

    // if the frontend if expecting short-term lambda trim
    // (as opposed to long-term trim)
    if (m_enabledSamples[SampleType_LambdaTrim] && (m_lambdaTrimType == C14CUX_LambdaTrimType_ShortTerm))
    {
        result = mergeResult(result, c14cux_getLambdaTrimShort(&m_cuxinfo, C14CUX_Bank_Left, &m_leftLambdaTrim));
        result = mergeResult(result, c14cux_getLambdaTrimShort(&m_cuxinfo, C14CUX_Bank_Right, &m_rightLambdaTrim));
    }

    if (m_enabledSamples[SampleType_EngineRPM])
        result = mergeResult(result, c14cux_getEngineRPM(&m_cuxinfo, &m_engineSpeedRPM));

    if (m_enabledSamples[SampleType_FuelMap])
    {
        result = mergeResult(result, c14cux_getFuelMapRowIndex(&m_cuxinfo, &m_currentFuelMapRowIndex));
        result = mergeResult(result, c14cux_getFuelMapColumnIndex(&m_cuxinfo, &m_currentFuelMapColumnIndex));
    }

    if (m_enabledSamples[SampleType_IdleBypassPosition])
        result = mergeResult(result, c14cux_getIdleBypassMotorPosition(&m_cuxinfo, &m_idleBypassPos));

    return result;
}

/**
 * Reads data that changes at a medium rate, such as lambda trim and target idle.
 * @return True if a read was scheduled and completed successfully,
 *  false otherwise.
 */
CUXInterface::ReadResult CUXInterface::readMidFreqData()
{
    ReadResult result = ReadResult_NoStatement;

    // if the frontend is expecting long-term lambda trim
    // (as opposed to short-term trim)
    if (m_enabledSamples[SampleType_LambdaTrim] && (m_lambdaTrimType == C14CUX_LambdaTrimType_LongTerm))
    {
        result = mergeResult(result, c14cux_getLambdaTrimLong(&m_cuxinfo, C14CUX_Bank_Left, &m_leftLambdaTrim));
        result = mergeResult(result, c14cux_getLambdaTrimLong(&m_cuxinfo, C14CUX_Bank_Right, &m_rightLambdaTrim));
    }

    if (m_enabledSamples[SampleType_MainVoltage])
        result = mergeResult(result, c14cux_getMainVoltage(&m_cuxinfo, &m_mainVoltage));

    if (m_enabledSamples[SampleType_TargetIdleRPM])
    {
        result = mergeResult(result, c14cux_getTargetIdle(&m_cuxinfo, &m_targetIdleSpeed));
        result = mergeResult(result, c14cux_getIdleMode(&m_cuxinfo, &m_idleMode));
    }

    if (m_enabledSamples[SampleType_FuelPumpRelay])
        result = mergeResult(result, c14cux_getFuelPumpRelayState(&m_cuxinfo, &m_fuelPumpRelayOn));

    if (m_enabledSamples[SampleType_GearSelection])
        result = mergeResult(result, c14cux_getGearSelection(&m_cuxinfo, &m_gear));

    if (m_enabledSamples[SampleType_RoadSpeed])
        result = mergeResult(result, c14cux_getRoadSpeed(&m_cuxinfo, &m_roadSpeedMPH));

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
    if (!c14cux_isMILOn(&m_cuxinfo, &m_milOn))
    {
        m_milOn = false;
    }

    // alternate between reading coolant temperature and fuel temperature
    if (m_enabledSamples[SampleType_EngineTemperature] && (m_readCount % 2 == 0))
        result = mergeResult(result, c14cux_getCoolantTemp(&m_cuxinfo, &m_coolantTempF));
    else if (m_enabledSamples[SampleType_FuelTemperature])
        result = mergeResult(result, c14cux_getFuelTemp(&m_cuxinfo, &m_fuelTempF));

    // less frequently, check the ID of the current fuel map
    // (this would only change as a result of a different
    //  tune resistor being switched in)
    if (m_enabledSamples[SampleType_FuelMap] && (m_readCount % 7 == 0))
        result = mergeResult(result, c14cux_getCurrentFuelMap(&m_cuxinfo, &m_currentFuelMapIndex));

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
    c14cux_cancelRead(&m_cuxinfo);
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
c14cux_gear CUXInterface::getGear()
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
c14cux_faultcodes CUXInterface::getFaultCodes()
{
    return m_faultCodes;
}

/**
 * Returns the last-read MIL status.
 * @return Last-read MIL status.
 */
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
QByteArray* CUXInterface::getFuelMap(unsigned int fuelMapId)
{
    QByteArray* map = 0;

    if (m_fuelMapDataIsCurrent[fuelMapId])
    {
        map = &(m_fuelMaps[fuelMapId]);
    }

    return map;
}

/**
 * Invalidates the stored fuel map data for the specified map ID,
 * forcing the data to be retrieved from the ECU again on the next
 * request.
 * @param ID of fuel map to invalidate
 */
void CUXInterface::invalidateFuelMapData(unsigned int fuelMapId)
{
    m_fuelMapDataIsCurrent[fuelMapId] = false;
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
QByteArray* CUXInterface::getROMImage()
{
    return m_romImage;
}

/**
 * Returns the last-read fuel map adjustment factor.
 * @return Last-read fuel map adjustment factor
 */
int CUXInterface::getFuelMapAdjustmentFactor(unsigned int fuelMapId)
{
    int adjFactor = -1;
    if (m_fuelMapDataIsCurrent[fuelMapId])
    {
        adjFactor = m_fuelMapAdjFactors[fuelMapId];
    }

    return adjFactor;
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
 * @param type Selects either Short- or Long-Term lambda trim.
 */
void CUXInterface::setLambdaTrimType(c14cux_lambda_trim_type type)
{
    m_lambdaTrimType = type;
}

/**
 * Sets the type of MAF reading to take (direct or linearized).
 * @param type Selects either Direct or Linearized MAF readings.
 */
void CUXInterface::setMAFReadingType(c14cux_airflow_type type)
{
    m_airflowType = type;
}

/**
 * Sets the type of throttle position reading to take (corrected or absolute).
 * @param type Selects either Absolute or Corrected throttle position type
 */
void CUXInterface::setThrottleReadingType(c14cux_throttle_pos_type type)
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

/**
 * Returns the last-read state of the idle-mode flag
 * @return Last-read idle-mode flag
 */
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

/**
 * Resets the long term lambda trim to the midpoint value
 */
void CUXInterface::onResetLongTermLambdaRequest()
{
    bool success = true;

    success &= c14cux_writeMem(&m_cuxinfo, C14CUX_LongTermLambdaFuelingTrimLeftOffset,   0x30);
    success &= c14cux_writeMem(&m_cuxinfo, C14CUX_LongTermLambdaFuelingTrimLeftOffset+1, 0x00);
    success &= c14cux_writeMem(&m_cuxinfo, C14CUX_LongTermLambdaFuelingTrimRightOffset,  0x20);
    success &= c14cux_writeMem(&m_cuxinfo, C14CUX_LongTermLambdaFuelingTrimRightOffset+1,0x00);
}

#ifdef ENABLE_SIM_MODE
void CUXInterface::onSimModeWriteRequest(bool enableSimMode, SimulationInputValues simVals, SimulationInputChanges changes)
{
    if (c14cux_connect(&m_cuxinfo, m_deviceName.toStdString().c_str()))
    {
        bool success = true;

        if (changes.inertiaSwitch)     success &= c14cux_writeMem(&m_cuxinfo, 0x2060, simVals.inertiaSwitch);
        if (changes.heatedScreen)      success &= c14cux_writeMem(&m_cuxinfo, 0x2061, simVals.heatedScreen);
        if (changes.maf)
        {
            success &= c14cux_writeMem(&m_cuxinfo, 0x2062, (uint8_t)((simVals.maf & (0xFF00)) >> 8));
            success &= c14cux_writeMem(&m_cuxinfo, 0x2063, (uint8_t)(simVals.maf & (0x00FF)));
        }
        if (changes.throttle)
        {
            success &= c14cux_writeMem(&m_cuxinfo, 0x2064, (uint8_t)((simVals.throttle & (0xFF00)) >> 8));
            success &= c14cux_writeMem(&m_cuxinfo, 0x2065, (uint8_t)(simVals.throttle & (0x00FF)));
        }
        if (changes.coolantTemp)       success &= c14cux_writeMem(&m_cuxinfo, 0x2066, simVals.coolantTemp);
        if (changes.neutralSwitch)     success &= c14cux_writeMem(&m_cuxinfo, 0x2067, simVals.neutralSwitch);
        if (changes.airConLoad)        success &= c14cux_writeMem(&m_cuxinfo, 0x2068, simVals.airConLoad);
        if (changes.roadSpeed)         success &= c14cux_writeMem(&m_cuxinfo, 0x2069, simVals.roadSpeed);
        if (changes.mainRelay)         success &= c14cux_writeMem(&m_cuxinfo, 0x206A, simVals.mainRelay);
        if (changes.mafTrim)           success &= c14cux_writeMem(&m_cuxinfo, 0x206B, simVals.mafTrim);
        if (changes.tuneResistor)      success &= c14cux_writeMem(&m_cuxinfo, 0x206C, simVals.tuneResistor);
        if (changes.fuelTemp)          success &= c14cux_writeMem(&m_cuxinfo, 0x206D, simVals.fuelTemp);
        if (changes.o2LeftDutyCycle)   success &= c14cux_writeMem(&m_cuxinfo, 0x206E, simVals.o2LeftDutyCycle);
        if (changes.o2SensorReference) success &= c14cux_writeMem(&m_cuxinfo, 0x206F, simVals.o2SensorReference);
        if (changes.diagnosticPlug)    success &= c14cux_writeMem(&m_cuxinfo, 0x2070, simVals.diagnosticPlug);
        if (changes.o2RightDutyCycle)  success &= c14cux_writeMem(&m_cuxinfo, 0x2071, simVals.o2RightDutyCycle);

        if (enableSimMode && success)
        {
            // write the magic pattern to turn on simulation mode
            success &= c14cux_writeMem(&m_cuxinfo, 0x2072, 0x55);

            // clear any fault codes that were set as a result of running the ECU
            // with sensors missing from the harness
            success &= c14cux_clearFaultCodes(&m_cuxinfo);
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
#endif

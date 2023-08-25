#include <QThread>
#include <QDateTime>
#include <QCoreApplication>
#include <string.h>
#include "cuxinterface.h"

/**
 * Constructor. Sets the serial device and measurement units.
 * @param device Name of (or path to) the serial device used to comminucate
 *  with the 14CUX.
 * @param baud Baud rate to use when communicatin with the ECU. Note that
 *  standard ECUs only support the standard rate of 7812.5 bps.
 * @param sUnits Units to be used when expressing road speed
 * @param tUnits Units to be used when expressing coolant/fuel temperature
 */
CUXInterface::CUXInterface(QString device, unsigned int baud, SpeedUnits sUnits,
                           TemperatureUnits tUnits, bool fuelMapRefresh, bool simulateConnection,
                           QObject* parent) :
  QObject(parent),
  m_sim(simulateConnection),
  m_simConnected(false),
  m_simEcu(nullptr),
  m_deviceName(device),
  m_baudRate(baud),
  m_stopPolling(false),
  m_shutdownThread(false),
  m_batteryBackedMem(21, 0x00),
  m_readCanceled(false),
  m_lambdaTrimType(C14CUX_LambdaTrimType_ShortTerm),
  m_feedbackMode(C14CUX_FeedbackMode_ClosedLoop),
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
  m_fuelMapIndexRead(false),
  m_currentFuelMapIndex(0),
  m_currentFuelMapRowIndex(0),
  m_fuelMapRowWeighting(0),
  m_currentFuelMapColumnIndex(0),
  m_fuelMapColWeighting(0),
  m_mafReading(0.0),
  m_idleBypassPos(0.0),
  m_fuelPumpRelayOn(false),
  m_lambdaTrimOdd(0),
  m_lambdaTrimEven(0),
  m_coTrimVoltage(0.0),
  m_milOn(false),
  m_idleMode(false),
  m_injectorPulseWidthUs(0),
  m_injectorPulseWidthMs(0.0),
  m_tune(0),
  m_checksumFixer(0),
  m_ident(0),
  m_romImage(16384, 0x00),
  m_speedUnits(sUnits),
  m_tempUnits(tUnits),
  m_fuelMapRefresh(fuelMapRefresh),
  m_initComplete(false),
  m_rpmLimitRead(false)
{
  for (unsigned int idx = 0; idx < fuelMapCount; ++idx)
  {
    m_fuelMaps[idx].fill(0x00, 128);
    m_fuelMapDataIsCurrent[idx] = false;
    m_rowScaler[idx] = 0;
  }

  memset(&m_rpmTable, 0, sizeof(m_rpmTable));

  for (int type = 0; type < (int)SampleType_NumSampleTypes; type++)
  {
    m_lastReadTime.insert((SampleType)type, 0);
  }

  if (m_sim)
  {
    m_simEcu = new SimulatedECUData();
  }
}

/**
 * Destructor.
 */
CUXInterface::~CUXInterface()
{
  delete m_simEcu;
}

/**
 * Enqueue a data request that requires a parameter (such as a fuel map ID,
 * or number of steps to move the IAC motor.)
 */
void CUXInterface::enqueueRequest(QueueableRequest req, int data)
{
  m_queueMutex.lock();
  m_reqQueue.enqueue(std::pair<QueueableRequest, int>(req, data));
  m_queueMutex.unlock();
}

/**
 * Enqueue a data request that does not require a parameter.
 */
void CUXInterface::enqueueRequest(QueueableRequest req)
{
  enqueueRequest(req, 0);
}

void CUXInterface::processQueuedRequest()
{
  if (isConnected())
  {
    m_queueMutex.lock();
    const std::pair<QueueableRequest, int> req = m_reqQueue.dequeue();
    m_queueMutex.unlock();

    const QueueableRequest reqType = std::get<0>(req);
    const int reqData = std::get<1>(req);

    switch (reqType)
    {
    case QueueableRequest_BatteryBackedMem:
      readBatteryBackedMem();
      break;
    case QueueableRequest_ClearFaultCodes:
      clearFaultCodes();
      break;
    case QueueableRequest_FaultCodes:
      readFaultCodes();
      break;
    case QueueableRequest_FuelMapData:
      readFuelMap(reqData);
      break;
    case QueueableRequest_IACMotorDrive:
      driveIACMotor(reqData);
      break;
    case QueueableRequest_ROMImage:
      readROMImage();
      break;
    case QueueableRequest_RPMTable:
      readRPMTable();
      break;
    case QueueableRequest_TuneRevID:
      readTuneRevID();
      break;
    default:
      // nop
      break;
    }
  }
  else
  {
    emit notConnected();
  }
}

/**
 * Reads fault codes data from the 14CUX and stores it in a member structure.
 * Note that this routine does not check for an existing connection to the ECU.
 */
void CUXInterface::readFaultCodes()
{
  memset(&m_faultCodes, 0, sizeof(m_faultCodes));
  if (m_sim || c14cux_getFaultCodes(&m_cuxinfo, &m_faultCodes))
  {
    emit faultCodesReady();
  }
  else
  {
    emit faultCodesReadFailed();
  }
}

/**
 * Reads battery-backed memory from the 14CUX and stores in a member structure
 */
void CUXInterface::readBatteryBackedMem()
{
  if (m_sim || c14cux_readMem(&m_cuxinfo, 0x0040, 21, reinterpret_cast<uint8_t*>(m_batteryBackedMem.data())))
  {
    emit batteryBackedMemReady();
  }
  else
  {
    emit batteryBackedMemReadFailed();
  }
}

/**
 * Clears the block of fault codes.
 */
void CUXInterface::clearFaultCodes()
{
  if (m_sim)
  {
    memset(&m_faultCodes, 0, sizeof(m_faultCodes));
    emit faultCodesClearSuccess(m_faultCodes);
  }
  else
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
}

/**
 * Reads the entire 16KB ROM.
 */
void CUXInterface::readROMImage()
{
  if (m_sim)
  {
    // In simulation mode, we don't support reading the ROM image.
    emit romImageReadFailed();
  }
  else
  {
    if (c14cux_dumpROM(&m_cuxinfo, reinterpret_cast<uint8_t*>(m_romImage.data())))
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
  }

  m_readCanceled = false;
}

/**
 * Reads the data for the specified fuel map from the ECU, emitting a signal
 * when done.
 * @param fuelMapId ID of the fuel map that should be retrieved (1 through 5)
 */
bool CUXInterface::readFuelMap(unsigned int fuelMapId)
{
  uint8_t* const buffer = reinterpret_cast<uint8_t* const>(m_fuelMaps[fuelMapId].data());
  uint16_t adjFactor = 0;
  bool status = false;

  if (m_sim)
  {
    m_simEcu->fuelMapData(buffer, m_mafScaler, m_fuelMapAdjFactors[fuelMapId]);
    m_fuelMapDataIsCurrent[fuelMapId] = true;
    status = true;
  }
  else if (c14cux_getFuelMap(&m_cuxinfo, static_cast<int8_t>(fuelMapId), &adjFactor, &m_rowScaler[fuelMapId], buffer) &&
           c14cux_readMem(&m_cuxinfo, C14CUX_MAFRowScalerOffset, 2, reinterpret_cast<uint8_t*>(&m_mafScaler)))
  {
    m_mafScaler = swapShort(m_mafScaler);
    m_fuelMapAdjFactors[fuelMapId] = adjFactor;
    m_fuelMapDataIsCurrent[fuelMapId] = true;
    status = true;
  }

  if (status)
  {
    emit fuelMapReady(fuelMapId);
  }

  return status;
}

/**
 * Reads the RPM table from the ECU, emitting a signal if successful.
 */
void CUXInterface::readRPMTable()
{
  if (m_sim)
  {
    m_simEcu->engineRPMTable(m_rpmTable);
    emit rpmTableReady();
  }
  else if (c14cux_getRpmTable(&m_cuxinfo, &m_rpmTable))
  {
    emit rpmTableReady();
  }
}

/**
 * Commands the ECU to close the fuel pump relay and run the fuel pump for
 * a single time interval.
 */
void CUXInterface::runFuelPump()
{
  if (m_sim)
  {
    // TODO: set fuel pump run flag
  }
  else
  {
    c14cux_runFuelPump(&m_cuxinfo);
  }
}

/**
 * Commands the ECU to move the idle air control valve.
 * @param steps Direction and distance of travel for the idle air control valve;
 *  positive to open and negative to close
 */
void CUXInterface::driveIACMotor(int steps)
{
  // this should be set to 0 when opening the valve, 1 when closing
  const uint8_t direction = (steps >= 0) ? 0 : 1;
  const uint8_t distance = abs(steps);
  if (!m_sim)
  {
    c14cux_driveIdleAirControlMotor(&m_cuxinfo, direction, distance);
  }
}

void CUXInterface::readTuneRevID()
{
  if (m_sim)
  {
    emit revisionNumberReady(1234, 255, 255);
  }
  else if (c14cux_getTuneRevision(&m_cuxinfo, &m_tune, &m_checksumFixer, &m_ident))
  {
    emit revisionNumberReady(m_tune, m_checksumFixer, m_ident);
  }
}

/**
 * Attempts to open the serial device that is connected to the 14CUX.
 * @return True if serial device was opened successfully; false otherwise.
 */
bool CUXInterface::connectToECU()
{
  if (m_sim)
  {
    m_simConnected = true;
  }

  const bool status =
    m_sim ? true : c14cux_connect(&m_cuxinfo, m_deviceName.toStdString().c_str(), m_baudRate);

  if (status)
  {
    emit connected();
  }

  return status;
}

/**
 * Sets a flag that will cause us to stop polling and disconnect from the serial device.
 */
void CUXInterface::disconnectFromECU()
{
  m_stopPolling = true;
}

/**
 * Clears flags and stored data. Called on disconnect, so that reconnecting will force
 * retrieval of fresh data from the ECU.
 */
void CUXInterface::clearFlagsAndData()
{
  // invalidate the stored fuel map data so that it is retrieved again upon reconnecting
  for (unsigned int idx = 0; idx < fuelMapCount; ++idx)
  {
    m_fuelMapDataIsCurrent[idx] = false;
  }

  m_fuelMapIndexRead = false;

  for (int type = 0; type < (int)SampleType_NumSampleTypes; type++)
  {
    m_lastReadTime[(SampleType)type] = 0;
  }

  m_cuxinfo.promRev = C14CUX_DataOffsets_Unset;
  m_cuxinfo.voltageFactorA = 0;
  m_cuxinfo.voltageFactorB = 0;
  m_cuxinfo.voltageFactorC = 0;
  m_rpmLimitRead = false;
}

/**
 * Cleans up and exits the worker thread.
 */
void CUXInterface::onShutdownThreadRequest()
{
  // If we're currently connected, just set a flag to let the polling loop
  // shut the thread down. Otherwise, shut it down here.
  if (m_sim)
  {
    if (m_simConnected)
    {
      m_shutdownThread = true;
    }
    else
    {
      QThread::currentThread()->quit();
    }
  }
  else
  {
    if (c14cux_isConnected(&m_cuxinfo))
    {
      m_shutdownThread = true;
    }
    else
    {
      QThread::currentThread()->quit();
    }
  }
}

/**
 * Indicates whether the serial device is currently open/connected.
 * @return True when the device is connected; false otherwise.
 */
bool CUXInterface::isConnected()
{
  return m_sim ?
         (m_initComplete && m_simConnected) :
         (m_initComplete && c14cux_isConnected(&m_cuxinfo));
}

/**
 * Responds to the parent thread being started by initializing the libcomm14cux
 * structure (if necessary), and emitting a signal indicating that the
 * interface is ready.
 */
void CUXInterface::onParentThreadStarted()
{
  // Initialize the interface state info struct here, so that
  // it's in the context of the thread that will use it.
  if (!m_initComplete)
  {
    if (!m_sim)
    {
      c14cux_init(&m_cuxinfo);
    }
    m_initComplete = true;
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
    runServiceLoop();
  }
  else
  {
#ifdef WIN32
    // workaround for legacy Windows nonsense
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
 * Calls readData() in a loop until commanded to disconnect and possibly
 * shut down the thread.
 */
void CUXInterface::runServiceLoop()
{
  ReadResult res = ReadResult_NoStatement;
  bool connected = m_sim ? m_simConnected : c14cux_isConnected(&m_cuxinfo);

  while (!m_stopPolling && !m_shutdownThread && connected)
  {
    // process any queued requests before we get the periodic data update
    while (!m_reqQueue.isEmpty())
    {
      processQueuedRequest();
    }

    res = m_sim ? readSimData() : readData();

    if (res == ReadResult_Success)
    {
      emit readSuccess();
      emit dataReady();
    }
    else if (res == ReadResult_Failure)
    {
      emit readError();
    }

    QCoreApplication::processEvents();
  }

  if (m_sim)
  {
    m_simConnected = false;
  }
  else if (connected)
  {
    c14cux_disconnect(&m_cuxinfo);
  }

  emit disconnected();
  clearFlagsAndData();

  if (m_shutdownThread)
  {
    QThread::currentThread()->quit();
  }
}

/**
 * Determines if the sample type should be read given the current operating mode
 */
bool CUXInterface::isSampleAppropriateForMode(SampleType type) const
{
  bool status = true;

  if (type == SampleType_LambdaTrimLong)
  {
    status = (m_feedbackMode == C14CUX_FeedbackMode_ClosedLoop) &&
             (m_lambdaTrimType == C14CUX_LambdaTrimType_LongTerm);
  }
  else if (type == SampleType_LambdaTrimShort)
  {
    status = (m_feedbackMode == C14CUX_FeedbackMode_ClosedLoop) &&
             (m_lambdaTrimType == C14CUX_LambdaTrimType_ShortTerm);
  }
  else if (type == SampleType_COTrimVoltage)
  {
    status = (m_feedbackMode == C14CUX_FeedbackMode_OpenLoop);
  }
  else if (type == SampleType_FuelMapData)
  {
    // only refresh the fuel map data itself if a special option is set
    status = m_fuelMapRefresh;
  }

  return status;
}

/**
 * Determines if a sample type is due to be read (i.e. enough time has passed since the
 * last reading to prevent another read from being redundant)
 */
bool CUXInterface::isDueForMeasurement(SampleType type)
{
  bool status = false;

  if (m_enabledSamples[type] && isSampleAppropriateForMode(type))
  {
    qint64 now = QDateTime::currentMSecsSinceEpoch();

    if (now - m_lastReadTime[type] >= m_readIntervals[type])
    {
      status = true;
      m_lastReadTime[type] = now;
    }
  }

  return status;
}

/**
 * Reads data from the 14CUX via calls to the library, and stores the data in
 * member variables.
 * @return True if at least one value was read successfully; false otherwise.
 */
CUXInterface::ReadResult CUXInterface::readData()
{
  ReadResult result = ReadResult_NoStatement;

  if (isDueForMeasurement(SampleType_MAF))
  {
    result = mergeResult(result, c14cux_getMAFReading(&m_cuxinfo, m_airflowType, &m_mafReading));
  }

  if (isDueForMeasurement(SampleType_Throttle))
  {
    result = mergeResult(result, c14cux_getThrottlePosition(&m_cuxinfo, m_throttlePosType, &m_throttlePos));
  }

  if (isDueForMeasurement(SampleType_LambdaTrimShort))
  {
    result = mergeResult(result, c14cux_getLambdaTrimShort(&m_cuxinfo, C14CUX_Bank_Odd, &m_lambdaTrimOdd));
    result = mergeResult(result, c14cux_getLambdaTrimShort(&m_cuxinfo, C14CUX_Bank_Even, &m_lambdaTrimEven));
  }

  if (isDueForMeasurement(SampleType_EngineRPM))
  {
    result = mergeResult(result, c14cux_getEngineRPM(&m_cuxinfo, &m_engineSpeedRPM));

    // If we haven't yet reported the RPM limit, see if we can read it now.
    // This is a special case because the limit is only read into its RAM
    // location in the ECU once the main spark interrupt has run; we therefore
    // wait until the engine speed > 0 before attempting this.
    if (!m_rpmLimitRead &&
        (result == ReadResult_Success) &&
        (m_engineSpeedRPM > 0) &&
        c14cux_getRPMLimit(&m_cuxinfo, &m_rpmLimit))
    {
      m_rpmLimitRead = true;
      emit rpmLimitReady(m_rpmLimit);
    }
  }

  if (isDueForMeasurement(SampleType_FuelMapRowCol))
  {
    result = mergeResult(result, c14cux_getFuelMapRowIndex(&m_cuxinfo, &m_currentFuelMapRowIndex, &m_fuelMapRowWeighting));
    result = mergeResult(result, c14cux_getFuelMapColumnIndex(&m_cuxinfo, &m_currentFuelMapColumnIndex, &m_fuelMapColWeighting));
  }

  if (isDueForMeasurement(SampleType_InjectorPulseWidth))
  {
    result = mergeResult(result, c14cux_getInjectorPulseWidth(&m_cuxinfo, &m_injectorPulseWidthUs));
    m_injectorPulseWidthMs = (float)m_injectorPulseWidthUs / 1000.0;
  }

  if (isDueForMeasurement(SampleType_IdleBypassPosition))
  {
    result = mergeResult(result, c14cux_getIdleBypassMotorPosition(&m_cuxinfo, &m_idleBypassPos));
  }

  if (isDueForMeasurement(SampleType_LambdaTrimLong))
  {
    result = mergeResult(result, c14cux_getLambdaTrimLong(&m_cuxinfo, C14CUX_Bank_Odd, &m_lambdaTrimOdd));
    result = mergeResult(result, c14cux_getLambdaTrimLong(&m_cuxinfo, C14CUX_Bank_Even, &m_lambdaTrimEven));
  }

  if (isDueForMeasurement(SampleType_MainVoltage))
  {
    result = mergeResult(result, c14cux_getMainVoltage(&m_cuxinfo, &m_mainVoltage));
  }

  if (isDueForMeasurement(SampleType_TargetIdleRPM))
  {
    result = mergeResult(result, c14cux_getTargetIdle(&m_cuxinfo, &m_targetIdleSpeed));
    result = mergeResult(result, c14cux_getIdleMode(&m_cuxinfo, &m_idleMode));
  }

  if (isDueForMeasurement(SampleType_FuelPumpRelay))
  {
    result = mergeResult(result, c14cux_getFuelPumpRelayState(&m_cuxinfo, &m_fuelPumpRelayOn));
  }

  if (isDueForMeasurement(SampleType_GearSelection))
  {
    result = mergeResult(result, c14cux_getGearSelection(&m_cuxinfo, &m_gear));
  }

  if (isDueForMeasurement(SampleType_RoadSpeed))
  {
    result = mergeResult(result, c14cux_getRoadSpeed(&m_cuxinfo, &m_roadSpeedMPH));
  }

  if (isDueForMeasurement(SampleType_EngineTemperature))
  {
    result = mergeResult(result, c14cux_getCoolantTemp(&m_cuxinfo, &m_coolantTempF));
  }

  if (isDueForMeasurement(SampleType_FuelTemperature))
  {
    result = mergeResult(result, c14cux_getFuelTemp(&m_cuxinfo, &m_fuelTempF));
  }

  if (isDueForMeasurement(SampleType_FuelMapData))
  {
    if (readFuelMap(m_currentFuelMapIndex))
    {
      emit fuelMapReady(m_currentFuelMapIndex);
    }
  }

  // attempt to read the MIL status; if it can't be read, default it to off on the display
  if (isDueForMeasurement(SampleType_MIL))
  {
    if (c14cux_isMILOn(&m_cuxinfo, &m_milOn))
    {
      result = mergeResult(result, true);
    }
    else
    {
      result = mergeResult(result, false);
      m_milOn = false;
    }
  }

  if (isDueForMeasurement(SampleType_FuelMapIndex))
  {
    uint8_t newFuelMapIndex = 0;
    bool fuelMapIndexReadResult = c14cux_getCurrentFuelMap(&m_cuxinfo, &newFuelMapIndex);
    result = mergeResult(result, fuelMapIndexReadResult);

    // do some processing that is only relevant if we successfully read the current map ID
    if (fuelMapIndexReadResult)
    {
      // if the fuel map index has changed, or if this is the first time we've read it
      if ((newFuelMapIndex != m_currentFuelMapIndex) || !m_fuelMapIndexRead)
      {
        m_currentFuelMapIndex = newFuelMapIndex;
        emit fuelMapIndexHasChanged(m_currentFuelMapIndex);
      }

      // regardless of whether the map has changed, we know now
      // that is has been read at least once
      m_fuelMapIndexRead = true;

      // set the current fueling mode (open-loop or closed-loop)
      c14cux_feedback_mode newFeedbackMode = C14CUX_FeedbackMode_ClosedLoop;

      if ((m_currentFuelMapIndex >= s_firstOpenLoopMap) &&
          (m_currentFuelMapIndex <= s_lastOpenLoopMap))
      {
        newFeedbackMode = C14CUX_FeedbackMode_OpenLoop;
      }

      // if the feedback mode has changed, emit a signal
      if (newFeedbackMode != m_feedbackMode)
      {
        m_feedbackMode = newFeedbackMode;
        emit feedbackModeHasChanged(m_feedbackMode);
      }
    }
  }

  if (isDueForMeasurement(SampleType_COTrimVoltage))
  {
    result = mergeResult(result, c14cux_getCOTrimVoltage(&m_cuxinfo, &m_coTrimVoltage));
  }

  return result;
}

CUXInterface::ReadResult CUXInterface::readSimData()
{
  if (isDueForMeasurement(SampleType_MAF))
  {
    QThread::currentThread()->msleep(5);
    m_mafReading = m_simEcu->maf();
  }

  if (isDueForMeasurement(SampleType_Throttle))
  {
    QThread::currentThread()->msleep(5);
    m_throttlePos = m_simEcu->throttle();
  }

  if (isDueForMeasurement(SampleType_LambdaTrimShort))
  {
    QThread::currentThread()->msleep(5);
    m_lambdaTrimOdd = m_simEcu->lambdaShortOdd();
    m_lambdaTrimEven = m_simEcu->lambdaShortEven();
  }

  if (isDueForMeasurement(SampleType_EngineRPM))
  {
    QThread::currentThread()->msleep(5);
    m_engineSpeedRPM = m_simEcu->engineRPM();
    m_rpmLimitRead = true;
    emit rpmLimitReady(m_simEcu->engineRPMLimit());
  }

  if (isDueForMeasurement(SampleType_FuelMapRowCol))
  {
    QThread::currentThread()->msleep(5);
    m_simEcu->fuelMapRowColIndices(m_currentFuelMapRowIndex, m_fuelMapRowWeighting,
                                   m_currentFuelMapColumnIndex, m_fuelMapColWeighting);
  }

  if (isDueForMeasurement(SampleType_InjectorPulseWidth))
  {
    QThread::currentThread()->msleep(5);
    m_injectorPulseWidthUs = m_simEcu->injectorPulsewidthUs();
    m_injectorPulseWidthMs = (float)m_injectorPulseWidthUs / 1000.0;
  }

  if (isDueForMeasurement(SampleType_IdleBypassPosition))
  {
    QThread::currentThread()->msleep(5);
    m_idleBypassPos = m_simEcu->idleBypassPos();
  }

  if (isDueForMeasurement(SampleType_LambdaTrimLong))
  {
    QThread::currentThread()->msleep(5);
    m_lambdaTrimOdd = m_simEcu->lambdaLongOdd();
    m_lambdaTrimEven = m_simEcu->lambdaLongEven();
  }

  if (isDueForMeasurement(SampleType_MainVoltage))
  {
    QThread::currentThread()->msleep(5);
    m_mainVoltage = m_simEcu->mainVoltage();
  }

  if (isDueForMeasurement(SampleType_TargetIdleRPM))
  {
    QThread::currentThread()->msleep(5);
    m_targetIdleSpeed = m_simEcu->targetIdle();
    m_idleMode = m_simEcu->idleMode();
  }

  if (isDueForMeasurement(SampleType_FuelPumpRelay))
  {
    QThread::currentThread()->msleep(5);
    m_fuelPumpRelayOn = m_simEcu->fuelPumpRelayState();
  }

  if (isDueForMeasurement(SampleType_GearSelection))
  {
    QThread::currentThread()->msleep(5);
    m_gear = (c14cux_gear)m_simEcu->gearSelection();
  }

  if (isDueForMeasurement(SampleType_RoadSpeed))
  {
    QThread::currentThread()->msleep(5);
    m_roadSpeedMPH = m_simEcu->roadSpeedMPH();
  }

  if (isDueForMeasurement(SampleType_EngineTemperature))
  {
    QThread::currentThread()->msleep(5);
    m_coolantTempF = m_simEcu->coolantTempF();
  }

  if (isDueForMeasurement(SampleType_FuelTemperature))
  {
    QThread::currentThread()->msleep(5);
    m_fuelTempF = m_simEcu->fuelTempF();
  }

  if (isDueForMeasurement(SampleType_FuelMapData))
  {
    if (readFuelMap(m_currentFuelMapIndex))
    {
      emit fuelMapReady(m_currentFuelMapIndex);
    }
  }

  // attempt to read the MIL status; if it can't be read, default it to off on the display
  if (isDueForMeasurement(SampleType_MIL))
  {
    QThread::currentThread()->msleep(5);
    m_milOn = m_simEcu->mil();
  }

  if (isDueForMeasurement(SampleType_FuelMapIndex))
  {
    QThread::currentThread()->msleep(5);
    uint8_t newFuelMapIndex = m_simEcu->currentFuelMap();

    // if the fuel map index has changed, or if this is the first time we've read it
    if ((newFuelMapIndex != m_currentFuelMapIndex) || !m_fuelMapIndexRead)
    {
      m_currentFuelMapIndex = newFuelMapIndex;
      emit fuelMapIndexHasChanged(m_currentFuelMapIndex);
    }

    // regardless of whether the map has changed, we know now
    // that is has been read at least once
    m_fuelMapIndexRead = true;

    // set the current fueling mode (open-loop or closed-loop)
    c14cux_feedback_mode newFeedbackMode = C14CUX_FeedbackMode_ClosedLoop;

    if ((m_currentFuelMapIndex >= s_firstOpenLoopMap) &&
        (m_currentFuelMapIndex <= s_lastOpenLoopMap))
    {
      newFeedbackMode = C14CUX_FeedbackMode_OpenLoop;
    }

    // if the feedback mode has changed, emit a signal
    if (newFeedbackMode != m_feedbackMode)
    {
      m_feedbackMode = newFeedbackMode;
      emit feedbackModeHasChanged(m_feedbackMode);
    }
  }

  if (isDueForMeasurement(SampleType_COTrimVoltage))
  {
    QThread::currentThread()->msleep(5);
    m_coTrimVoltage = m_simEcu->coTrimVoltage();
  }

  return CUXInterface::ReadResult_Success;
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
  if (!m_sim)
  {
    c14cux_cancelRead(&m_cuxinfo);
  }
}

/**
 * Returns the data for a particular fuel map.
 * @param fuelMapId ID of the fuel map to retrieve
 * @return Pointer to the container holding the fuel map data, or nullptr if the
 *   fuel map in question has not yet been retrieved
 */
const QByteArray* const CUXInterface::getFuelMap(unsigned int fuelMapId) const
{
  return (m_fuelMapDataIsCurrent[fuelMapId] ? &m_fuelMaps[fuelMapId] : nullptr);
}

/**
 * Invalidates the stored fuel map data for the specified map ID,
 * forcing the data to be retrieved from the ECU again on the next
 * request.
 * @param ID of fuel map to invalidate
 */
void CUXInterface::invalidateFuelMapData()
{
  for (unsigned int idx = 0; idx < fuelMapCount; ++idx)
  {
    m_fuelMapDataIsCurrent[idx] = false;
  }
}

/**
 * Returns the last-read fuel map adjustment factor.
 * @return Last-read fuel map adjustment factor
 */
int CUXInterface::getFuelMapAdjustmentFactor(unsigned int fuelMapId) const
{
  return (m_fuelMapDataIsCurrent[fuelMapId] ? m_fuelMapAdjFactors[fuelMapId] : -1);
}

/**
 * Converts speed in miles per hour to the desired units.
 * @param speedMph Speed in miles per hour
 * @return Speed in the desired units
 */
unsigned int CUXInterface::convertSpeed(unsigned int speedMph) const
{
  float speed = (float)speedMph;

  if (m_speedUnits == KPH)
  {
    speed *= 1.609344;
  }

  return (unsigned int)speed;
}

/**
 * Converts temperature in Fahrenheit degrees to the desired units.
 * @param tempF Temperature in Fahrenheit degrees
 * @return Temperature in the desired units
 */
int CUXInterface::convertTemperature(int tempF) const
{
  double temp = tempF;

  switch (m_tempUnits)
  {
  case Celsius:
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
void CUXInterface::setEnabledSamples(QMap<SampleType, bool> samples)
{
  // the fields are updated one at a time, because a replacement of the entire
  // hash table (using the assignment operator) can disrupt other threads that
  // are reading the table at that time
  foreach(SampleType field, samples.keys())
  {
    m_enabledSamples[field] = samples[field];
  }

  zeroDisabledSamples();
}

/**
 * For the samples that are disabled, set the stored last reading to a default
 * value (zero) so that they do not appear as valid-but-unchanging data points
 * in the log file.
 */
void CUXInterface::zeroDisabledSamples()
{
  if (!m_enabledSamples[SampleType_EngineTemperature])
  {
    m_coolantTempF = 0;
  }

  if (!m_enabledSamples[SampleType_RoadSpeed])
  {
    m_roadSpeedMPH = 0;
  }

  if (!m_enabledSamples[SampleType_EngineRPM])
  {
    m_engineSpeedRPM = 0;
  }

  if (!m_enabledSamples[SampleType_FuelTemperature])
  {
    m_fuelTempF = 0;
  }

  if (!m_enabledSamples[SampleType_MAF])
  {
    m_mafReading = 0.0;
  }

  if (!m_enabledSamples[SampleType_Throttle])
  {
    m_throttlePos = 0.0;
  }

  if (!m_enabledSamples[SampleType_IdleBypassPosition])
  {
    m_idleBypassPos = 0.0;
  }

  if (!m_enabledSamples[SampleType_TargetIdleRPM])
  {
    m_targetIdleSpeed = 0;
  }

  if (!m_enabledSamples[SampleType_GearSelection])
  {
    m_gear = C14CUX_Gear_NoReading;
  }

  if (!m_enabledSamples[SampleType_MainVoltage])
  {
    m_mainVoltage = 0.0;
  }

  if (!m_enabledSamples[SampleType_COTrimVoltage])
  {
    m_coTrimVoltage = 0.0;
  }

  if (!m_enabledSamples[SampleType_FuelPumpRelay])
  {
    m_fuelPumpRelayOn = false;
  }

  if (!m_enabledSamples[SampleType_InjectorPulseWidth])
  {
    m_injectorPulseWidthUs = 0.0;
    m_injectorPulseWidthMs = 0.0;
  }

  if (!m_enabledSamples[SampleType_FuelMapRowCol])
  {
    m_currentFuelMapRowIndex = 0;
    m_currentFuelMapColumnIndex = 0;
  }

  if ((!m_enabledSamples[SampleType_LambdaTrimShort] && (m_lambdaTrimType == C14CUX_LambdaTrimType_ShortTerm)) ||
      (!m_enabledSamples[SampleType_LambdaTrimLong]  && (m_lambdaTrimType == C14CUX_LambdaTrimType_LongTerm)))
  {
    m_lambdaTrimOdd = 0;
    m_lambdaTrimEven = 0;
  }
}

/**
 * Updates the list of intervals at which the various sensor values should be read
 */
void CUXInterface::setReadIntervals(QHash<SampleType, unsigned int> intervals)
{
  foreach(SampleType field, intervals.keys())
  {
    m_readIntervals[field] = intervals[field];
  }
}


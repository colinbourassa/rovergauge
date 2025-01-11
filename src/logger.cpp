#include <QtGlobal>
#include <QDir>
#include <QDateTime>
#include "logger.h"

/**
 * Constructor. Sets the 14CUX interface class pointer as
 * well as log directory and log file extension.
 */
Logger::Logger(CUXInterface& cuxIFace, OptionsDialog& options) :
  m_cux(cuxIFace),
  m_options(options),
  m_logExtension(".txt"),
  m_logDir("logs")
{
}

/**
 * Attempts to open a log file with the name specified.
 * @return True on success, false otherwise
 */
bool Logger::openLog(QString fileName)
{
  bool success = false;
  unsigned int fmRow = 0;
  unsigned int fmCol = 0;
  bool alreadyExists = false;

  m_lastAttemptedLog = m_logDir + QDir::separator() + fileName + m_logExtension;
  m_lastAttemptedStaticLog = m_logDir + QDir::separator() + fileName + "_static" + m_logExtension;

  // if the 'logs' directory exists, or if we're able to create it...
  if (!m_logFile.isOpen() && (QDir(m_logDir).exists() || QDir().mkdir(m_logDir)))
  {
    // set the name of the log file and open it for writing
    alreadyExists = QFileInfo(m_lastAttemptedLog).exists();
    m_logFile.setFileName(m_lastAttemptedLog);

    if (m_logFile.open(QFile::WriteOnly | QFile::Append))
    {
      m_logFileStream.setDevice(&m_logFile);

      if (!alreadyExists)
      {
        m_logFileStream << "#datetime,roadSpeed,engineSpeed,waterTemp,fuelTemp," <<
                        "throttlePos,mafPercentage,idleBypassPos,mainVoltage," <<
                        "currentFuelMapIndex,currentFuelMapRow,currentFuelMapCol," <<
                        "targetIdle,lambdaTrimOdd,lambdaTrimEven,pulseWidthMs" << Qt::endl;
      }

      success = true;
    }

    // if that worked, attempt to open a file for the static/one-shot data
    if (success)
    {
      alreadyExists = QFileInfo(m_lastAttemptedStaticLog).exists();
      m_staticLogFile.setFileName(m_lastAttemptedStaticLog);

      if (m_staticLogFile.open(QFile::WriteOnly | QFile::Append))
      {
        m_staticLogFileStream.setDevice(&m_staticLogFile);

        if (!alreadyExists)
        {
          m_staticDataLogged = false;
          m_staticLogFileStream << "#datetime,tune,ident,checksumFixer,fuelMapIndex," <<
                                "fuelMapMultiplier,rowScaler,rowOffset,mafCOTrim";

          // give each byte of the fuel map a separate field name
          for (fmRow = 0; fmRow < FUEL_MAP_ROWS; fmRow += 1)
          {
            for (fmCol = 0; fmCol < FUEL_MAP_COLUMNS; fmCol += 1)
            {
              m_staticLogFileStream << QString(",FM_R%1C%2").arg(fmRow).arg(fmCol);
            }
          }

          m_staticLogFileStream << Qt::endl;
        }
      }
    }
  }

  return success;
}

/**
 * Close the log file(s).
 */
void Logger::closeLog()
{
  m_logFile.close();
  m_staticLogFile.close();
}

/**
 * Commands the logger to query the 14CUX interface for the currently
 * buffered data, and write it to the file.
 */
void Logger::logData()
{
  // One of two flags that must be set to allow logging of static data.
  // This one keeps track of the receipt of firmware build identifiers (tune ID, etc.)
  // and the other keeps track of the receipt of actual fuel map data.
  m_miscStaticDataIsReady = true;

  if (m_logFile.isOpen() && (m_logFileStream.status() == QTextStream::Ok))
  {
    double roadSpeed = m_cux.getRoadSpeed();

    if (m_options.getSpeedoAdjust())
    {
      roadSpeed *= m_options.getSpeedoMultiplier();
      roadSpeed += m_options.getSpeedoOffset();
    }

    m_logFileStream << getTimestamp(false) << ","
                    << roadSpeed << ","
                    << m_cux.getEngineSpeedRPM() << ","
                    << m_cux.getCoolantTemp() << ","
                    << m_cux.getFuelTemp() << ","
                    << m_cux.getThrottlePos() << ","
                    << m_cux.getMAFReading() << ","
                    << m_cux.getIdleBypassPos() << ","
                    << m_cux.getMainVoltage() << ","
                    << m_cux.getCurrentFuelMapIndex() << ","
                    << getRowWithWeighting() << ","
                    << getColWithWeighting() << ","
                    << m_cux.getTargetIdleSpeed() << ","
                    << m_cux.getLambdaTrimOdd() << ","
                    << m_cux.getLambdaTrimEven() << ","
                    << m_cux.getInjectorPulseWidthMs()
                    << Qt::endl;
  }

  if (!m_staticDataLogged &&
      m_fuelMapDataIsReady &&
      m_staticLogFile.isOpen() &&
      (m_staticLogFileStream.status() == QTextStream::Ok))
  {
    logStaticData(m_fuelMapId);
  }
}

/**
 * Gets the timestamp string used when writing a log entry.
 * Depending on settings, the time will either represent an absolute time or
 * a delta time (against the time of the first log entry.)
 */
QString Logger::getTimestamp(bool forStaticData)
{
  if (!m_timeOfFirstDataSet)
  {
    m_timeOfFirstData = QDateTime::currentDateTime();
    m_timeOfFirstDataSet = true;
  }

  QString timestampStr;

  if (m_options.logTimesMsecsFromZero())
  {
    if (forStaticData)
    {
      // Because the static data does not reflect parametrics that change dynamically
      // while the engine is running, we will always log it with a timestamp of 0
      // milliseconds in this mode.
      timestampStr = "0";
    }
    else
    {
      // For dynamic data, log it with the displacement in milliseconds from the
      // first dynamic data log entry.
      timestampStr = QString::number(m_timeOfFirstData.msecsTo(QDateTime::currentDateTime()));
    }
  }
  else
  {
    timestampStr = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh:mm:ss.zzz");
  }

  return timestampStr;
}

/**
 * Writes a single entry in a 'static data' log for elements that will likely
 * not change (tune ID, ident byte, fuel map content, etc.)
 */
void Logger::logStaticData(unsigned int fuelMapId)
{
  if (m_staticLogLock.tryLock())
  {
    m_staticDataLogged = true;

    const QByteArray* fuelMapData = m_cux.getFuelMap(fuelMapId);
    float mafCoTrim = 0.0;
    unsigned char c;

    // only get the MAF CO trim if an open-loop map is selected
    if ((fuelMapId > 0) && (fuelMapId < 4))
    {
      mafCoTrim = m_cux.getCOTrimVoltage();
    }

    m_staticLogFileStream << getTimestamp(true) << ","
                          << Qt::uppercasedigits
                          << m_cux.getTune() << ","
                          << Qt::hex << m_cux.getIdent() << ","
                          << Qt::hex << m_cux.getChecksumFixer() << ","
                          << Qt::dec << fuelMapId << ","
                          << Qt::hex << m_cux.getFuelMapAdjustmentFactor(fuelMapId) << ","
                          << Qt::hex << m_cux.getRowScaler(fuelMapId) << ","
                          << m_cux.getMAFRowScaler() << ","
                          << mafCoTrim;

    if (fuelMapData)
    {
      // write out every byte of the fuel map data
      for (unsigned int fmRow = 0; fmRow < FUEL_MAP_ROWS; fmRow += 1)
      {
        for (unsigned int fmCol = 0; fmCol < FUEL_MAP_COLUMNS; fmCol += 1)
        {
          c = fuelMapData->at(fmRow * FUEL_MAP_COLUMNS + fmCol);
          m_staticLogFileStream << "," << QString::number(c, 16).toUpper();
        }
      }
    }

    m_staticLogFileStream << Qt::endl;
  }
}

/**
 * Slot called when the interface class has finished reading the fuel map data
 * for the provided fuel map ID.
 */
void Logger::onFuelMapDataReady(unsigned int fuelMapId)
{
  m_fuelMapDataIsReady = true;
  m_fuelMapId = fuelMapId;

  // If a log file has already been opened, then log the static data now.
  // Otherwise, set a flag that can be checked if/when a log is ultimately
  // opened.
  if (!m_staticDataLogged &&
      m_miscStaticDataIsReady && // check that the other static data is actually ready
      m_staticLogFile.isOpen() &&
      (m_staticLogFileStream.status() == QTextStream::Ok))
  {
    logStaticData(fuelMapId);
  }
}

/**
 * Gets a fractional value that describes the current fuel map row index considering
 * the weighting.
 */
float Logger::getRowWithWeighting() const
{
  return ((float)m_cux.getFuelMapRowIndex() +
          ((float)m_cux.getFuelMapRowWeighting() / 16.0));
}

/**
 * Gets a fractional value that describes the current fuel map column index considering
 * the weighting.
 */
float Logger::getColWithWeighting() const
{
  return ((float)m_cux.getFuelMapColumnIndex() +
          ((float)m_cux.getFuelMapColumnWeighting() / 16.0));
}

/**
 * Returns the full path to the last log that we attempted to open.
 * @return Full path to last log file
 */
QString Logger::getLogPath()
{
  return m_lastAttemptedLog;
}

/**
 * Clears some flags that change the logging behavior for static data
 */
void Logger::onDisconnect()
{
  m_fuelMapDataIsReady = false;
  m_fuelMapId = 0;
  m_staticDataLogged = false;
}


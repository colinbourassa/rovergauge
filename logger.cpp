#include <QDir>
#include <QDateTime>
#include "logger.h"

/**
 * Constructor. Sets the 14CUX interface class pointer as
 * well as log directory and log file extension.
 */
Logger::Logger(CUXInterface *cuxIFace) :
m_fuelMapDataIsReady(false),
m_fuelMapId(0),
m_logExtension(".txt"),
m_logDir("logs")
{
    m_cux = cuxIFace;
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

  m_lastAttemptedLog = m_logDir + QDir::separator() + fileName + m_logExtension;
  m_lastAttemptedStaticLog = m_logDir + QDir::separator() + fileName + "_static" + m_logExtension;

  // if the 'logs' directory exists, or if we're able to create it...
  if (!m_logFile.isOpen() && (QDir(m_logDir).exists() || QDir().mkdir(m_logDir)))
  {
    // set the name of the log file and open it for writing
    bool alreadyExists = QFileInfo(m_lastAttemptedLog).exists();
    m_logFile.setFileName(m_lastAttemptedLog);
    if (m_logFile.open(QFile::WriteOnly | QFile::Append))
    {
      m_logFileStream.setDevice(&m_logFile);

      if (!alreadyExists)
      {
        m_logFileStream << "#datetime,roadSpeed,engineSpeed,waterTemp,fuelTemp," <<
          "throttlePos,mafPercentage,idleBypassPos,mainVoltage," <<
          "currentFuelMapIndex,currentFuelMapRow,currentFuelMapCol," <<
          "targetIdle,lambdaTrimOdd,lambdaTrimEven,pulseWidthMs" << endl;
      }

      success = true;
    }

    // if that worked, attempt to open a file for the static/one-shot data
    if (success)
    {
      m_staticLogFile.setFileName(m_lastAttemptedStaticLog);

      if (m_staticLogFile.open(QFile::WriteOnly | QFile::Truncate))
      {
        m_staticLogFileStream.setDevice(&m_staticLogFile);
        m_staticLogFileStream << "#datetime,tune,ident,checksumFixer,fuelMapIndex,fuelMapMultiplier,rowScalar,mafCOTrim";

        // give each byte of the fuel map a separate field name
        for (fmRow = 0; fmRow < FUEL_MAP_ROWS; fmRow += 1)
        {
          for (fmCol = 0; fmCol < FUEL_MAP_COLUMNS; fmCol += 1)
          {
            m_staticLogFileStream << QString(",FM_R%1C%2").arg(fmRow).arg(fmCol);
          }
        }

        m_staticLogFileStream << endl;
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
  if (m_logFile.isOpen() && (m_logFileStream.status() == QTextStream::Ok))
  {
    m_logFileStream << QDateTime::currentDateTime().toString("yyyy-MM-dd_hh:mm:ss.zzz") << ","
      << m_cux->getRoadSpeed() << ","
      << m_cux->getEngineSpeedRPM() << ","
      << m_cux->getCoolantTemp() << ","
      << m_cux->getFuelTemp() << ","
      << m_cux->getThrottlePos() << ","
      << m_cux->getMAFReading() << ","
      << m_cux->getIdleBypassPos() << ","
      << m_cux->getMainVoltage() << ","
      << m_cux->getCurrentFuelMapIndex() << ","
      << getRowWithWeighting() << ","
      << getColWithWeighting() << ","
      << m_cux->getTargetIdleSpeed() << ","
      << m_cux->getLambdaTrimOdd() << ","
      << m_cux->getLambdaTrimEven() << ","
      << m_cux->getInjectorPulseWidthMs()
      << endl;
  }

  if (m_fuelMapDataIsReady &&
      m_staticLogFile.isOpen() &&
      (m_staticLogFileStream.status() == QTextStream::Ok))
  {
    logStaticData(m_fuelMapId);
  }
}

/**
 * Writes a single entry in a 'static data' log for elements that will likely
 * not change (tune ID, ident byte, fuel map content, etc.)
 */
void Logger::logStaticData(unsigned int fuelMapId)
{
  QByteArray* fuelMapData = m_cux->getFuelMap(fuelMapId);
  float mafCoTrim = 0.0;
  unsigned char c;

  // only get the MAF CO trim if an open-loop map is selected
  if ((fuelMapId > 0) && (fuelMapId < 4))
  {
    mafCoTrim = m_cux->getCOTrimVoltage();
  }

  m_staticLogFileStream << QDateTime::currentDateTime().toString("yyyy-MM-dd_hh:mm:ss.zzz") << ","
    << m_cux->getTune() << ","
    << hex << m_cux->getIdent() << ","
    << hex << m_cux->getChecksumFixer() << ","
    << dec << fuelMapId << ","
    << m_cux->getFuelMapAdjustmentFactor(fuelMapId) << ","
    << m_cux->getRowScalar() << ","
    << mafCoTrim;

  if (fuelMapData)
  {
    // write out every byte of the fuel map data
    for (unsigned int fmRow = 0; fmRow < FUEL_MAP_ROWS; fmRow += 1)
    {
      for (unsigned int fmCol = 0; fmCol < FUEL_MAP_COLUMNS; fmCol += 1)
      {
        c = fuelMapData->at(fmRow * FUEL_MAP_COLUMNS + fmCol);
        m_staticLogFileStream << "," << QString::number(c);
      }
    }
  }

  m_staticLogFileStream << endl;
  m_staticLogFile.close();
}

/**
 * Slot called when the interface class has finished reading the fuel map data
 * for the provided fuel map ID.
 */
void Logger::onFuelMapDataReady(unsigned int fuelMapId)
{
  // If a log file has already been opened, then log the static data now.
  // Otherwise, set a flag that can be checked if/when a log is ultimately
  // opened.
  if (m_staticLogFile.isOpen() && (m_staticLogFileStream.status() == QTextStream::Ok))
  {
    logStaticData(fuelMapId);
  }
  else
  {
    m_fuelMapDataIsReady = true;
    m_fuelMapId = fuelMapId;
  }
}

/**
 * Gets a fractional value that describes the current fuel map row index considering
 * the weighting.
 */
float Logger::getRowWithWeighting()
{
  return ((float)m_cux->getFuelMapRowIndex() +
          ((float)m_cux->getFuelMapRowWeighting() / 16.0));
}

/**
 * Gets a fractional value that describes the current fuel map column index considering
 * the weighting.
 */
float Logger::getColWithWeighting()
{
  return ((float)m_cux->getFuelMapColumnIndex() +
          ((float)m_cux->getFuelMapColWeighting() / 16.0));
}

/**
 * Returns the full path to the last log that we attempted to open.
 * @return Full path to last log file
 */
QString Logger::getLogPath()
{
    return m_lastAttemptedLog;
}


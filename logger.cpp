#include <QDir>
#include <QDateTime>
#include "logger.h"

/**
 * Constructor. Sets the 14CUX interface class pointer as
 * well as log directory and log file extension.
 */
Logger::Logger(CUXInterface *cuxIFace) :
    m_logExtension(".txt"), m_logDir("logs")
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

        if (success)
        {
          m_staticLogFile.setFileName(m_lastAttemptedStaticLog);

          if (m_staticLogFile.open(QFile::WriteOnly | QFile::Truncate))
          {
            m_staticLogFileStream.setDevice(&m_staticLogFile);
            m_staticLogFileStream << "#datetime,tune,ident,fuelMapIndex,fuelMapMultiplier,rowScalar,mafCOTrim";

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
 * Close the log file.
 */
void Logger::closeLog()
{
    m_logFile.close();
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


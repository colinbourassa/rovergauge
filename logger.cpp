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

    m_lastAttemptedLog = m_logDir + QDir::separator() + fileName + m_logExtension;

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
                m_logFileStream << "#time,roadSpeed,engineSpeed,waterTemp,fuelTemp," <<
                                 "throttlePos,mafPercentage,idleBypassPos,mainVoltage," <<
                                 "currentFuelMapIndex,currentFuelMapRow,currentFuelMapCol," <<
                                 "targetIdle,leftLambdaTrim,rightLambdaTrim" << endl;
            }

            success = true;
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
        m_logFileStream << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << ","
                      << m_cux->getRoadSpeed() << ","
                      << m_cux->getEngineSpeedRPM() << ","
                      << m_cux->getCoolantTemp() << ","
                      << m_cux->getFuelTemp() << ","
                      << m_cux->getThrottlePos() << ","
                      << m_cux->getMAFReading() << ","
                      << m_cux->getIdleBypassPos() << ","
                      << m_cux->getMainVoltage() << ","
                      << m_cux->getCurrentFuelMapIndex() << ","
                      << m_cux->getFuelMapRowIndex() << ","
                      << m_cux->getFuelMapColumnIndex() << ","
                      << m_cux->getTargetIdleSpeed() << ","
                      << m_cux->getLeftLambdaTrim() << ","
                      << m_cux->getRightLambdaTrim()
                      << endl;
    }
}

/**
 * Returns the full path to the last log that we attempted to open.
 * @return Full path to last log file
 */
QString Logger::getLogPath()
{
    return m_lastAttemptedLog;
}


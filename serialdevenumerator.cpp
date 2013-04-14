#include <QStringList>
#ifdef linux
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#elif defined(WIN32)
#include <stdio.h>
#include <windows.h>
#endif
#include "serialdevenumerator.h"

/**
 * Constructor.
 */
SerialDevEnumerator::SerialDevEnumerator()
{
}

/**
 * Queries the OS / device filesystem for the list of serial devices likely
 * to be the one that is connected to the 14CUX ECU.
 * @return List of device names
 */
QStringList SerialDevEnumerator::getSerialDevList(QString savedDevName)
{
    QStringList serialDevices;
    if (savedDevName.isNull() || savedDevName.isEmpty())
    {
        serialDevices.append("");
    }
    else
    {
        serialDevices.append(savedDevName);
    }

#ifdef linux

    // first check to see if this Linux distribution uses the /dev/serial/
    // directory to store symlinks to each serial device
    QDir devSerial("/dev/serial/by-id/", "", QDir::Name, QDir::Files | QDir::NoDotAndDotDot);
    if (devSerial.exists())
    {
        QFileInfoList files = devSerial.entryInfoList();
        foreach (const QFileInfo file, files)
        {
            if (file.exists())
            {
                if (file.isSymLink())
                {
                    serialDevices.append(file.symLinkTarget());
                }
                else
                {
                    serialDevices.append(file.absoluteFilePath());
                }
            }
        }
    }

    // if nothing was found using the method above, simply return a list of
    // devices that match the pattern "ttyS*"
    if (serialDevices.count() == 0)
    {
        QDir dev("/dev", "ttyUSB* ttyS*", QDir::NoSort,
                 QDir::Files | QDir::System | QDir::Hidden | QDir::NoDotAndDotDot);
        QFileInfoList files = dev.entryInfoList();
        foreach (const QFileInfo file, files)
        {
            if (file.exists())
            {
                if (file.isSymLink())
                {
                    serialDevices.append(file.symLinkTarget());
                }
                else
                {
                    serialDevices.append(file.absoluteFilePath());
                }
            }
        }
    }

#elif defined(__APPLE__)

    QDir dev("/dev", "cu.usbserial*", QDir::NoSort,
            QDir::Files | QDir::System | QDir::Hidden | QDir::NoDotAndDotDot);
    QFileInfoList files = dev.entryInfoList();
    foreach (const QFileInfo file, files)
    {
        if (file.exists())
        {
            if (file.isSymLink())
            {
                serialDevices.append(file.symLinkTarget());
            }
            else
            {
                serialDevices.append(file.absoluteFilePath());
            }
        }
    }

#elif defined(WIN32)

    // compiling with MinGW, so use a WinAPI mechanism to enumerate ports
    WCHAR portName[8];
    COMMCONFIG cc;
    DWORD dwSize = sizeof(COMMCONFIG);

    // apparently, COM ports can be numbered from 1 to 255
    for (int portNum = 1; portNum < 256; portNum++)
    {
        _snwprintf(portName, 8, L"COM%d", portNum);
        if (GetDefaultCommConfig(portName, &cc, &dwSize))
        {
            serialDevices.append(QString::fromWCharArray(portName));
        }
    }

#endif

    serialDevices.removeDuplicates();
    return serialDevices;
}


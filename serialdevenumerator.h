#ifndef SERIALDEVENUMERATOR_H
#define SERIALDEVENUMERATOR_H

#include <QStringList>

class SerialDevEnumerator
{
public:
    SerialDevEnumerator();
    QStringList getSerialDevList(QString savedDevName);
};

#endif // SERIALDEVENUMERATOR_H


#ifndef SERIALDEVENUMERATOR_H
#define SERIALDEVENUMERATOR_H

#include <QStringList>

class SerialDevEnumerator
{
public:
    SerialDevEnumerator();
    QStringList getSerialDevList();
};

#endif // SERIALDEVENUMERATOR_H


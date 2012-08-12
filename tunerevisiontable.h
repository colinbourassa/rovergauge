#ifndef TUNEREVISIONTABLE_H
#define TUNEREVISIONTABLE_H

#include <QString>
#include <QHash>

class TuneRevisionTable
{
public:
    TuneRevisionTable();
    QString lookup(QString hashStr);

private:
    QHash<QString,QString> table;
};

#endif // TUNEREVISIONTABLE_H

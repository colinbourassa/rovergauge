#include "tunerevisiontable.h"

TuneRevisionTable::TuneRevisionTable()
{
    table.insert("60AC72A82163E0947CFCB5E8007A5A50", "R2103");
    table.insert("0824F8F5578D0A3D4BE500E6EF4F5351", "R2419");
    table.insert("7487B382CB894C968176C9C6D0B6D96C", "R2665");
    table.insert("B522819EE55DFE989179740F2B87DE32", "R3360");
    table.insert("010AD76837345AA2558CE1E150AF5999", "R3361");
    table.insert("60A635F76108D55C23494979081EE4A9", "Adams/Tornado");
}

QString TuneRevisionTable::lookup(QString md5sum)
{
    return table.value(md5sum, "(Unknown)");
}

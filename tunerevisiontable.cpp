#include "tunerevisiontable.h"

TuneRevisionTable::TuneRevisionTable()
{
    table.insert("60AC72A82163E0947CFCB5E8007A5A50", "R2103");
    table.insert("0824F8F5578D0A3D4BE500E6EF4F5351", "R2419");
    table.insert("7487B382CB894C968176C9C6D0B6D96C", "R2665");
    table.insert("14D6D79A9F8503BF506E96BB3E858FAA", "R3360");
    table.insert("FEED649CC9B6CFD6C95C968399457F6E", "R3361");
    table.insert("57DC1366A2E106AF68BE326FCA76B226", "R3365");
    table.insert("AE4B1F848ABDA66F31210D9E0D19D0ED", "R3526 (3360A)");
    table.insert("1515F31DF689522570A7C3485FABCAD2", "R3652");
    table.insert("9FF1515A0A8F667807E99F09E77E8A9B", "TVR");
    table.insert("60A635F76108D55C23494979081EE4A9", "(Aftermarket)");
}

QString TuneRevisionTable::lookup(QString md5sum)
{
    return table.value(md5sum, "");
}


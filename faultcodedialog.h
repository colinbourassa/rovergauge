#ifndef FAULTCODEDIALOG_H
#define FAULTCODEDIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QPushButton>
#include <QMap>
#include <QLabel>
#include <QString>
#include "comm14cux.h"
#include <qledindicator/qledindicator.h>

/**
 * Enumeration of fault codes used by the 14CUX.
 */
enum FaultCode
{
    FaultCode_ECUMemoryCheck      = 0,
    FaultCode_LambdaSensorLeft    = 1,
    FaultCode_LambdaSensorRight   = 2,
    FaultCode_MisfireLeft         = 3,
    FaultCode_MisfireRight        = 4,
    FaultCode_AirflowMeter        = 5,
    FaultCode_InjectorLeft        = 6,
    FaultCode_InjectorRight       = 7,
    FaultCode_CoolantTempSensor   = 8,
    FaultCode_ThrottlePot         = 9,
    FaultCode_ThrottlePotHiMAFLo  = 10,
    FaultCode_ThrottlePotLoMAFHi  = 11,
    FaultCode_PurgeValveLeak      = 12,
    FaultCode_IdleStepper         = 13,
    FaultCode_RoadSpeedSensor     = 14,
    FaultCode_NeutralSwitch       = 15,
    FaultCode_FuelTempSensor      = 16,
    FaultCode_BatteryDisconnected = 17,
    FaultCode_ECMMemoryCleared    = 18,
    FaultCode_TotalCount          = 19
};

/**
 * A dialog box populated with lamps that are lit when their corresponding
 *  fault code is on.
 */
class FaultCodeDialog : public QDialog
{
    Q_OBJECT

public:
    FaultCodeDialog(QString title, Comm14CUXFaultCodes faults);
    ~FaultCodeDialog();

signals:
    void clearFaultCodes();

public slots:
    void onFaultClearSuccess(Comm14CUXFaultCodes faultCodes);
    void onFaultClearFailure();

protected:
    void accept();

private:
    QGridLayout *grid;
    QPushButton *closeButton;
    QPushButton *clearButton;

    QMap<FaultCode, QString> faultNames;
    QMap<FaultCode, QLedIndicator*> faultLights;
    QMap<FaultCode, QLabel*> faultLabels;

    const int rows;

    void populateFaultList();
    void setupWidgets();
    void lightLEDs(Comm14CUXFaultCodes faults);
};

#endif // FAULTCODEDIALOG_H


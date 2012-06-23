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
    FaultCode_TuneResistor        = 6,
    FaultCode_InjectorLeft        = 7,
    FaultCode_InjectorRight       = 8,
    FaultCode_CoolantTempSensor   = 9,
    FaultCode_ThrottlePot         = 10,
    FaultCode_ThrottlePotHiMAFLo  = 11,
    FaultCode_ThrottlePotLoMAFHi  = 12,
    FaultCode_PurgeValveLeak      = 13,
    FaultCode_MixtureTooLean      = 14,
    FaultCode_IntakeAirLeak       = 15,
    FaultCode_LowFuelPressure     = 16,
    FaultCode_IdleStepper         = 17,
    FaultCode_RoadSpeedSensor     = 18,
    FaultCode_NeutralSwitch       = 19,
    FaultCode_FuelPressureOrLeak  = 20,
    FaultCode_FuelTempSensor      = 21,
    FaultCode_BatteryDisconnected = 22,
    FaultCode_ECMMemoryCleared    = 23,
    FaultCode_TotalCount          = 24
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


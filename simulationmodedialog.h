#ifndef SIMULATIONMODEDIALOG_H
#define SIMULATIONMODEDIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QSlider>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include "stdint.h"

typedef struct
{
    uint8_t roadSpeed;
    uint8_t airConLoad;
    uint16_t airflowA;
    uint16_t airflowB;
    uint16_t airflowC;
    uint16_t throttle;
    uint8_t coolantTemp;
    uint8_t fuelTemp;
    uint8_t o2SensorReference;
    uint8_t mainRelay;
    uint8_t inertiaSwitch;
    uint8_t neutralSwitch;
    uint8_t heatedScreen;
    uint8_t diagnosticPlug;
    uint8_t tuneResistor;

} SimulationInputValues;

class SimulationModeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SimulationModeDialog(const QString title, QWidget *parent = 0);

signals:
    void writeSimulationInputValues(SimulationInputValues values);

private:
    QGridLayout *grid;

    QLabel *inertiaSwitchLabel;
    QLabel *heatedScreenLabel;
    QLabel *airflowALabel;
    QLabel *airflowBLabel;
    QLabel *airflowCLabel;
    QLabel *throttlePositionLabel;
    QLabel *coolantTempLabel;
    QLabel *fuelTempLabel;
    QLabel *neutralSwitchLabel;
    QLabel *airConLoadLabel;
    QLabel *roadSpeedLabel;
    QLabel *mainRelayLabel;
    QLabel *tuneResistorLabel;
    QLabel *o2SensorReferenceLabel;
    QLabel *diagnosticPlugLabel;

    QLineEdit *inertiaSwitchVal;
    QLineEdit *heatedScreenVal;
    QLineEdit *airflowAVal;
    QLineEdit *airflowBVal;
    QLineEdit *airflowCVal;
    QLineEdit *throttlePositionVal;
    QLineEdit *coolantTempVal;
    QLineEdit *fuelTempVal;
    QLineEdit *neutralSwitchVal;
    QLineEdit *airConLoadVal;
    QLineEdit *roadSpeedVal;
    QLineEdit *mainRelayVal;
    QLineEdit *tuneResistorVal;
    QLineEdit *o2SensorReferenceVal;
    QLineEdit *diagnosticPlugVal;

    QCheckBox *inertiaSwitchBox;
    QCheckBox *neutralSwitchBox;
    QCheckBox *heatedScreenBox;
    QSlider *coolantTempSlider;
    QSlider *fuelTempSlider;

    QPushButton *writeButton;
    QPushButton *closeButton;

    void setupWidgets();

private slots:
    void onWriteClicked();
    void onCloseClicked();
};

#endif // SIMULATIONMODEDIALOG_H

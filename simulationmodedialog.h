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
#include <QComboBox>
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

    QLabel *inertiaSwitchVal;
    QLabel *heatedScreenVal;
    QLabel *airflowAVal;
    QLabel *airflowBVal;
    QLabel *airflowCVal;
    QLabel *throttlePositionVal;
    QLabel *coolantTempVal;
    QLabel *fuelTempVal;
    QLabel *neutralSwitchVal;
    QLabel *airConLoadVal;
    QLabel *roadSpeedVal;
    QLabel *mainRelayVal;
    QLabel *tuneResistorVal;
    QLabel *o2SensorReferenceVal;
    QLabel *diagnosticPlugVal;

    QLineEdit *inertiaSwitchRawVal;
    QLineEdit *heatedScreenRawVal;
    QLineEdit *airflowARawVal;
    QLineEdit *airflowBRawVal;
    QLineEdit *airflowCRawVal;
    QLineEdit *throttlePositionRawVal;
    QLineEdit *coolantTempRawVal;
    QLineEdit *fuelTempRawVal;
    QLineEdit *neutralSwitchRawVal;
    QLineEdit *airConLoadRawVal;
    QLineEdit *roadSpeedRawVal;
    QLineEdit *mainRelayRawVal;
    QLineEdit *tuneResistorRawVal;
    QLineEdit *o2SensorReferenceRawVal;
    QLineEdit *diagnosticPlugRawVal;

    QCheckBox *inertiaSwitchBox;
    QCheckBox *heatedScreenBox;
    QComboBox *neutralSwitchBox;
    QSlider *coolantTempSlider;
    QSlider *fuelTempSlider;
    QSlider *roadSpeedSlider;
    QSlider *throttleSlider;
    QSlider *mainRelaySlider;

    QPushButton *writeButton;
    QPushButton *closeButton;

    void setupWidgets();
    double Peak_LorentzianModifiedPeakG_model(double x_in);

private slots:
    void onWriteClicked();
    void onCloseClicked();

    void onInertiaSwitchChanged(bool checked);
    void onHeatedScreenChanged(bool checked);
    void onNeutralSwitchChanged(int val);
    void onRoadSpeedChanged(int val);
    void onCoolantTempChanged(int val);
    void onFuelTempChanged(int val);
    void onThrottleChanged(int val);
    void onMainRelayVoltageChanged(int val);
};

#endif // SIMULATIONMODEDIALOG_H

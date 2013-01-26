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
#include <QHBoxLayout>
#include "commonunits.h"

class SimulationModeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SimulationModeDialog(const QString title, QWidget *parent = 0);

signals:
    void writeSimulationInputValues(bool enableSimMode, SimulationInputValues values);

private:
    QGridLayout *grid;
    QHBoxLayout *buttonLayout;

    QLabel *inertiaSwitchLabel;
    QLabel *heatedScreenLabel;
    QLabel *mafLabel;
    QLabel *mafTrimLabel;
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
    QLabel *o2LeftDutyLabel;
    QLabel *o2RightDutyLabel;

    QLabel *inertiaSwitchVal;
    QLabel *heatedScreenVal;
    QLabel *mafVal;
    QLabel *mafTrimVal;
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
    QLabel *o2LeftDutyVal;
    QLabel *o2RightDutyVal;

    QLineEdit *inertiaSwitchRawVal;
    QLineEdit *heatedScreenRawVal;
    QLineEdit *mafRawVal;
    QLineEdit *mafTrimRawVal;
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
    QLineEdit *o2LeftDutyRawVal;
    QLineEdit *o2RightDutyRawVal;

    QCheckBox *inertiaSwitchBox;
    QCheckBox *heatedScreenBox;
    QCheckBox *airConLoadBox;
    QCheckBox *diagnosticPlugBox;
    QComboBox *neutralSwitchBox;
    QSlider *mafSlider;
    QSlider *mafTrimSlider;
    QSlider *coolantTempSlider;
    QSlider *fuelTempSlider;
    QSlider *roadSpeedSlider;
    QSlider *throttleSlider;
    QSlider *mainRelaySlider;
    QSlider *o2LeftDutySlider;
    QSlider *o2RightDutySlider;

    QPushButton *enableSimModeButton;
    QPushButton *writeButton;
    QPushButton *closeButton;

    void setupWidgets();
    double Peak_LorentzianModifiedPeakG_model(double x_in);
    unsigned int convertVoltageToQuadraticCounts(float voltage);
    void doWrite(bool enableSimMode);

private slots:
    void onEnabledSimModeClicked();
    void onWriteClicked();
    void onCloseClicked();

    void onWriteSuccess();
    void onWriteFailure();

    void onMafChanged(int val);
    void onMafTrimChanged(int val);
    void onInertiaSwitchChanged(bool checked);
    void onHeatedScreenChanged(bool checked);
    void onNeutralSwitchChanged(int val);
    void onRoadSpeedChanged(int val);
    void onCoolantTempChanged(int val);
    void onFuelTempChanged(int val);
    void onThrottleChanged(int val);
    void onMainRelayVoltageChanged(int val);
    void onAirConLoadChanged(bool checked);
    void onDiagnosticPlugChanged(bool checked);
    void onO2LeftDutyChanged(int val);
    void onO2RightDutyChanged(int val);
};

#endif // SIMULATIONMODEDIALOG_H

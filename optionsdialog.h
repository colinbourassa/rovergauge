#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QString>
#include <QMap>
#include <QFrame>
#include "commonunits.h"

enum SampleType
{
    SampleType_MAF,
    SampleType_Throttle,
    SampleType_IdleBypassPosition,
    SampleType_TargetIdleRPM,
    SampleType_GearSelection,
    SampleType_MainVoltage,
    SampleType_LambdaTrim,
    SampleType_CurrentFuelMap,
    SampleType_FuelMapRow,
    SampleType_FuelMapColumn,
    SampleType_FuelPumpRelay,
    SampleType_EngineTemperature,
    SampleType_RoadSpeed,
    SampleType_EngineRPM,
    SampleType_FuelTemperature
};

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    OptionsDialog(QString title, QWidget *parent = 0);
    QString getSerialDeviceName();
    bool getSerialDeviceChanged();
    int getSpeedMax();
    int getRedline();
    SpeedUnits getSpeedUnits();
    TemperatureUnits getTemperatureUnits();
    QMap<SampleType,bool> getEnabledSamples();

protected:
    void accept();

private slots:
    void checkAll();
    void uncheckAll();

private:
    QGridLayout *grid;
    QLabel *serialDeviceLabel;
    QComboBox *serialDeviceBox;
    QLabel *speedMaxLabel;
    QSpinBox *speedMaxBox;
    QLabel *redlineLabel;
    QSpinBox *redlineBox;

    QLabel *temperatureUnitsLabel;
    QComboBox *temperatureUnitsBox;

    QLabel *speedUnitsLabel;
    QComboBox *speedUnitsBox;

    QFrame *horizontalLineA;
    QFrame *horizontalLineB;
    QLabel *enabledSamplesLabel;
    QPushButton *checkAllButton;
    QPushButton *uncheckAllButton;
    QMap<SampleType,QCheckBox*> enabledSamplesBoxes;

    QPushButton *okButton;
    QPushButton *cancelButton;

    QString serialDeviceName;
    int speedMax;
    int redline;
    TemperatureUnits tempUnits;
    SpeedUnits speedUnits;

    QMap<SampleType,bool> enabledSamples;
    QMap<SampleType,QString> sampleTypeNames;
    QMap<SampleType,QString> sampleTypeLabels;
    bool serialDeviceChanged;

    const QString settingsFileName;
    const QString settingsGroupName;

    const QString settingSerialDev;
    const QString settingSpeedMax;
    const QString settingRedline;
    const QString settingSpeedUnits;
    const QString settingTemperatureUnits;

    void setupWidgets();
    void readSettings();
    void writeSettings();
};

#endif // OPTIONSDIALOG_H


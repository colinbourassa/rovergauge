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
#include <QHash>
#include <QFrame>
#include "commonunits.h"

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
    QHash<SampleType,bool> getEnabledSamples();

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
    QHash<SampleType,QCheckBox*> enabledSamplesBoxes;

    QPushButton *okButton;
    QPushButton *cancelButton;

    QString serialDeviceName;
    int speedMax;
    int redline;
    TemperatureUnits tempUnits;
    SpeedUnits speedUnits;

    QHash<SampleType,bool> enabledSamples;
    QHash<SampleType,QString> sampleTypeNames;
    QHash<SampleType,QString> sampleTypeLabels;
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


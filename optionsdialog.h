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

enum SpeedUnits
{
    MPH = 0,
    FPS = 1,
    KPH = 2,
    Knots = 3
};

enum TemperatureUnits
{
    Fahrenheit = 0,
    Rankine    = 1,
    Celcius    = 2,
    Kelvin     = 3,
    Reaumur    = 4,
    Delisle    = 5,
    Newton     = 6,
    Romer      = 7
};

class OptionsDialog : public QDialog
{
public:
    OptionsDialog(QString title);
    QString getSerialDeviceName();
    bool getSerialDeviceChanged();
    int getPollIntervalMilliseconds();
    bool getPollIntervalChanged();
    int getSpeedMax();
    int getRedline();
    SpeedUnits getSpeedUnits();
    TemperatureUnits getTemperatureUnits();

protected:
    void accept();

private:
    QGridLayout *grid;
    QLabel *serialDeviceLabel;
    QComboBox *serialDeviceBox;
    QLabel *pollIntervalLabel;
    QSpinBox *pollIntervalBox;
    QLabel *speedMaxLabel;
    QSpinBox *speedMaxBox;
    QLabel *redlineLabel;
    QSpinBox *redlineBox;

    QLabel *temperatureUnitsLabel;
    QComboBox *temperatureUnitsBox;

    QLabel *speedUnitsLabel;
    QComboBox *speedUnitsBox;

    QPushButton *okButton;
    QPushButton *cancelButton;

    QString serialDeviceName;
    int pollIntervalMilliseconds;
    int speedMax;
    int redline;
    TemperatureUnits tempUnits;
    SpeedUnits speedUnits;

    bool serialDeviceChanged;
    bool pollIntervalChanged;

    const QString settingsFileName;
    const QString settingsGroupName;

    const QString settingSerialDev;
    const QString settingPollIntervalMSecs;
    const QString settingSpeedMax;
    const QString settingRedline;
    const QString settingSpeedUnits;
    const QString settingTemperatureUnits;

    void setupWidgets();
    void readSettings();
    void writeSettings();
};

#endif // OPTIONSDIALOG_H


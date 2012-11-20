#include <QSettings>
#include "optionsdialog.h"
#include "serialdevenumerator.h"

/**
 * Constructor; sets up the options-dialog UI and sets settings-file field names.
 */
OptionsDialog::OptionsDialog(QString title, QWidget *parent) : QDialog(parent),
    serialDeviceChanged(false),
    settingsGroupName("Settings"),
    settingSerialDev("SerialDevice"),
    settingSpeedMax("SpeedometerMax"),
    settingRedline("Redline"),
    settingSpeedUnits("SpeedUnits"),
    settingTemperatureUnits("TemperatureUnits")
{
    this->setWindowTitle(title);
    readSettings();
    setupWidgets();
}

/**
 * Instantiates widgets, connects to their signals, and places them on the form.
 */
void OptionsDialog::setupWidgets()
{
    int row = 0;

    grid = new QGridLayout(this);

    serialDeviceLabel = new QLabel("Serial device name:", this);
    serialDeviceBox = new QComboBox(this);

    speedMaxLabel = new QLabel("Maximum speed on speedometer:", this);
    speedMaxBox = new QSpinBox(this);

    redlineLabel = new QLabel("Tachometer redline:", this);
    redlineBox = new QSpinBox(this);

    speedUnitsLabel = new QLabel("Speed units:", this);
    speedUnitsBox = new QComboBox(this);

    temperatureUnitsLabel = new QLabel("Temperature units:", this);
    temperatureUnitsBox = new QComboBox(this);

    okButton = new QPushButton("OK", this);
    cancelButton = new QPushButton("Cancel", this);

    SerialDevEnumerator serialDevs;
    serialDeviceBox->addItems(serialDevs.getSerialDevList(serialDeviceName));
    serialDeviceBox->setEditable(true);
    serialDeviceBox->setMinimumWidth(150);

    speedMaxBox->setMaximum(250);
    speedMaxBox->setMinimum(10);
    speedMaxBox->setValue(speedMax);

    speedUnitsBox->setEditable(false);
    speedUnitsBox->addItem("MPH");
    speedUnitsBox->addItem("ft/s");
    speedUnitsBox->addItem("km/h");
    speedUnitsBox->setCurrentIndex((int)speedUnits);

    temperatureUnitsBox->setEditable(false);
    temperatureUnitsBox->addItem("Fahrenheit");
    temperatureUnitsBox->addItem("Celcius");
    temperatureUnitsBox->setCurrentIndex((int)tempUnits);

    redlineBox->setMinimum(0);
    redlineBox->setMaximum(12000);
    redlineBox->setSingleStep(500);
    redlineBox->setValue(redline);

    grid->addWidget(serialDeviceLabel, row, 0);
    grid->addWidget(serialDeviceBox, row++, 1);

    grid->addWidget(speedMaxLabel, row, 0);
    grid->addWidget(speedMaxBox, row++, 1);

    grid->addWidget(redlineLabel, row, 0);
    grid->addWidget(redlineBox, row++, 1);

    grid->addWidget(speedUnitsLabel, row, 0);
    grid->addWidget(speedUnitsBox, row++, 1);

    grid->addWidget(temperatureUnitsLabel, row, 0);
    grid->addWidget(temperatureUnitsBox, row++, 1);

    grid->addWidget(okButton, row, 0);
    grid->addWidget(cancelButton, row++, 1);

    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

/**
 * Reads the new settings from the form controls.
 */
void OptionsDialog::accept()
{
    QString newSerialDeviceName = serialDeviceBox->currentText();

    // set a flag if the serial device has been changed;
    // the main application needs to know if it should
    // reconnect to the 14CUX
    if (serialDeviceName.compare(newSerialDeviceName) != 0)
    {
        serialDeviceName = newSerialDeviceName;
        serialDeviceChanged = true;
    }
    else
    {
        serialDeviceChanged = false;
    }

    speedMax = speedMaxBox->value();
    redline = redlineBox->value();
    tempUnits = (TemperatureUnits)(temperatureUnitsBox->currentIndex());
    speedUnits = (SpeedUnits)(speedUnitsBox->currentIndex());

    writeSettings();
    done(QDialog::Accepted);
}

/**
 * Reads values for all the settings (either from the settings file, or by return defaults.)
 */
void OptionsDialog::readSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "RoverGauge");

    settings.beginGroup(settingsGroupName);
    serialDeviceName = settings.value(settingSerialDev, "").toString();
    speedMax = settings.value(settingSpeedMax, 160).toInt();
    redline = settings.value(settingRedline, 5500).toInt();
    speedUnits = (SpeedUnits)(settings.value(settingSpeedUnits, MPH).toInt());
    tempUnits = (TemperatureUnits)(settings.value(settingTemperatureUnits, Fahrenheit).toInt());

    settings.endGroup();
}

/**
 * Writes settings out to a file on disk.
 */
void OptionsDialog::writeSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "RoverGauge");

    settings.beginGroup(settingsGroupName);
    settings.setValue(settingSerialDev, serialDeviceName);
    settings.setValue(settingSpeedMax, speedMax);
    settings.setValue(settingRedline, redline);
    settings.setValue(settingSpeedUnits, speedUnits);
    settings.setValue(settingTemperatureUnits, tempUnits);

    settings.endGroup();
}

/**
 * Returns a flag indicating whether the serial-device name has been changed.
 */
bool OptionsDialog::getSerialDeviceChanged()
{
    return serialDeviceChanged;
}

/**
 * Returns the name of the serial device.
 */
QString OptionsDialog::getSerialDeviceName()
{
#ifdef WIN32
    return QString("\\\\.\\%1").arg(serialDeviceName);
#else
    return serialDeviceName;
#endif
}

/**
 * Returns the maximum value to be shown on the speedometer.
 */
int OptionsDialog::getSpeedMax()
{
    return speedMax;
}

/**
 * Returns the redline to be shown on the tachometer.
 */
int OptionsDialog::getRedline()
{
    return redline;
}

/**
 * Returns the units used for the speedometer
 */
SpeedUnits OptionsDialog::getSpeedUnits()
{
    return speedUnits;
}

/**
 * Returns the units used for the temperature gauges
 */
TemperatureUnits OptionsDialog::getTemperatureUnits()
{
    return tempUnits;
}

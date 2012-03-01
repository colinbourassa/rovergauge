#include <QSettings>
#include "optionsdialog.h"
#include "serialdevenumerator.h"

/**
 * Constructor; sets up the options-dialog UI and sets settings-file field names.
 */
OptionsDialog::OptionsDialog() :
    serialDeviceChanged(false),
    settingsFileName("settings.ini"),
    settingsGroupName("Settings"),
    settingSerialDev("SerialDevice"),
    settingPollIntervalMSecs("PollIntervalMilliseconds"),
    settingSpeedMax("SpeedometerMaxMPH")
{
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

    pollIntervalLabel = new QLabel("Poll interval in milliseconds:", this);
    pollIntervalBox = new QSpinBox(this);

    speedMaxLabel = new QLabel("Maximum speed on speedometer:", this);
    speedMaxBox = new QSpinBox(this);

    okButton = new QPushButton("OK", this);
    cancelButton = new QPushButton("Cancel", this);

    SerialDevEnumerator serialDevs;
    serialDeviceBox->addItems(serialDevs.getSerialDevList(serialDeviceName));
    serialDeviceBox->setEditable(true);
    serialDeviceBox->setMinimumWidth(150);

    pollIntervalBox->setMaximum(60000);
    pollIntervalBox->setMinimum(0);
    pollIntervalBox->setValue(pollIntervalMilliseconds);

    speedMaxBox->setMaximum(250);
    speedMaxBox->setMinimum(10);
    speedMaxBox->setValue(speedMax);

    grid->addWidget(serialDeviceLabel, row, 0);
    grid->addWidget(serialDeviceBox, row++, 1);

    grid->addWidget(pollIntervalLabel, row, 0);
    grid->addWidget(pollIntervalBox, row++, 1);

    grid->addWidget(speedMaxLabel, row, 0);
    grid->addWidget(speedMaxBox, row++, 1);

    grid->addWidget(okButton, row, 1);
    grid->addWidget(cancelButton, row++, 2);

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

    // caller would probably also like to know whether it
    // should change its poll timer interval
    if (pollIntervalMilliseconds != pollIntervalBox->value())
    {
        pollIntervalMilliseconds = pollIntervalBox->value();
        pollIntervalChanged = true;
    }
    else
    {
        pollIntervalChanged = false;
    }

    speedMax = speedMaxBox->value();

    writeSettings();
    done(QDialog::Accepted);
}

/**
 * Reads values for all the settings (either from the settings file, or by return defaults.)
 */
void OptionsDialog::readSettings()
{
    QSettings settings(settingsFileName, QSettings::IniFormat);

    settings.beginGroup(settingsGroupName);
    serialDeviceName = settings.value(settingSerialDev, "").toString();
    pollIntervalMilliseconds = settings.value(settingPollIntervalMSecs, 500).toInt();
    speedMax = settings.value(settingSpeedMax, 160).toInt();

    settings.endGroup();
}

/**
 * Writes settings out to a file on disk.
 */
void OptionsDialog::writeSettings()
{
    QSettings settings(settingsFileName, QSettings::IniFormat);

    settings.beginGroup(settingsGroupName);
    settings.setValue(settingSerialDev, serialDeviceName);
    settings.setValue(settingPollIntervalMSecs, pollIntervalMilliseconds);
    settings.setValue(settingSpeedMax, speedMax);

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
 * Returns a flag indicating whether the poll interval has been changed.
 */
bool OptionsDialog::getPollIntervalChanged()
{
    return pollIntervalChanged;
}

/**
 * Returns the poll interval in seconds.
 */
int OptionsDialog::getPollIntervalMilliseconds()
{
    return pollIntervalMilliseconds;
}

/**
 * Returns the name of the serial device.
 */
QString OptionsDialog::getSerialDeviceName()
{
    return serialDeviceName;
}

/**
 * Returns the maximum value to be shown on the speedometer.
 */
int OptionsDialog::getSpeedMax()
{
    return speedMax;
}

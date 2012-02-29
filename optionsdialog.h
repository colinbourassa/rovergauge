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

class OptionsDialog : public QDialog
{
public:
    OptionsDialog();
    QString getSerialDeviceName();
    bool getSerialDeviceChanged();
    int getPollIntervalMilliseconds();
    bool getPollIntervalChanged();
    int getSpeedMax();

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

    QPushButton *okButton;
    QPushButton *cancelButton;

    QString serialDeviceName;
    int pollIntervalMilliseconds;
    int speedMax;

    bool serialDeviceChanged;
    bool pollIntervalChanged;

    const QString settingsFileName;
    const QString settingsGroupName;

    const QString settingSerialDev;
    const QString settingPollIntervalMSecs;
    const QString settingSpeedMax;

    void setupWidgets();
    void readSettings();
    void writeSettings();
};

#endif // OPTIONSDIALOG_H


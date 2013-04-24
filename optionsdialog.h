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
    QGridLayout *m_grid;
    QLabel *m_serialDeviceLabel;
    QComboBox *m_serialDeviceBox;

    QLabel *m_temperatureUnitsLabel;
    QComboBox *m_temperatureUnitsBox;

    QLabel *m_speedUnitsLabel;
    QComboBox *m_speedUnitsBox;

    QFrame *m_horizontalLineA;
    QFrame *m_horizontalLineB;
    QLabel *m_enabledSamplesLabel;
    QPushButton *m_checkAllButton;
    QPushButton *m_uncheckAllButton;
    QHash<SampleType,QCheckBox*> m_enabledSamplesBoxes;

    QPushButton *m_okButton;
    QPushButton *m_cancelButton;

    QString m_serialDeviceName;
    TemperatureUnits m_tempUnits;
    SpeedUnits m_speedUnits;

    QHash<SampleType,bool> m_enabledSamples;
    QHash<SampleType,QString> m_sampleTypeNames;
    QHash<SampleType,QString> m_sampleTypeLabels;
    bool m_serialDeviceChanged;

    const QString m_settingsFileName;
    const QString m_settingsGroupName;

    const QString m_settingSerialDev;
    const QString m_settingRedline;
    const QString m_settingSpeedUnits;
    const QString m_settingTemperatureUnits;

    void setupWidgets();
    void readSettings();
    void writeSettings();
};

#endif // OPTIONSDIALOG_H


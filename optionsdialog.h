#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QString>
#include <QHash>
#include "commonunits.h"

namespace Ui
{
    class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    OptionsDialog(QString title, QWidget *parent = 0);
    QString getSerialDeviceName();
    unsigned int getBaudRate()                        { return m_baudRate; }
    bool getSerialDeviceChanged()                     { return m_serialDeviceChanged; }
    bool getRefreshFuelMap()                          { return m_refreshFuelMap; }
    bool getSoftHighlight()                           { return m_softHighlight; }
    SpeedUnits getSpeedUnits()                        { return m_speedUnits; }
    TemperatureUnits getTemperatureUnits()            { return m_tempUnits; }
    QMap<SampleType,bool> getEnabledSamples()         { return m_enabledSamples; }
    QHash<SampleType,unsigned int> getReadIntervals() { return m_readIntervalsMs; }

protected:
    void accept();
    void reject();

private slots:
    void checkAll();
    void uncheckAll();

private:
    Ui::OptionsDialog *m_ui;

    QMap<SampleType,QCheckBox*> m_enabledSamplesBoxes;

    QString m_serialDeviceName;
    unsigned int m_baudRate;
    TemperatureUnits m_tempUnits;
    SpeedUnits m_speedUnits;

    QMap<SampleType,bool> m_enabledSamples;
    QMap<SampleType,QString> m_sampleTypeNames;
    QMap<SampleType,QString> m_sampleTypeLabels;
    QHash<SampleType,unsigned int> m_readIntervalsMs;
    bool m_serialDeviceChanged;
    bool m_refreshFuelMap;
    bool m_softHighlight;

    const QString m_settingsFileName;
    const QString m_settingsGroupName;

    const QString m_settingSerialDev;
    const QString m_settingRefreshFuelMap;
    const QString m_settingSoftHighlight;
    const QString m_settingSpeedUnits;
    const QString m_settingTemperatureUnits;

    void groupLikeSettings();
    void setupWidgets();
    void setWidgetValues();
    void readSettings();
    void writeSettings();
};

#endif // OPTIONSDIALOG_H


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QLabel>
#include <QFile>
#include <QLineEdit>
#include <QTextStream>
#include <QThread>
#include <QFrame>
#include <QTableWidget>
#include <QHash>
#include <QMap>
#include <QPair>
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <analogwidgets/manometer.h>
#include <qledindicator/qledindicator.h>
#include "optionsdialog.h"
#include "idleaircontroldialog.h"
#include "cuxinterface.h"
#include "aboutbox.h"
#include "logger.h"
#include "fueltrimbar.h"
#include "commonunits.h"
#include "helpviewer.h"
#ifdef ENABLE_SIM_MODE
    #include "simulationmodedialog.h"
#endif

#define NUM_ACTIVE_FUEL_MAP_CELLS 4

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{    
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void onDataReady();
    void onConnect();
    void onDisconnect();
    void onReadError();
    void onReadSuccess();
    void onFailedToConnect(QString dev);
    void onFaultCodesReady();
    void onFaultCodesReadFailed();
    void onFuelMapDataReady(unsigned int fuelMapId);
    void onTuneRevisionReady(int tuneRevisionNum, int checksumFixer, int ident);
    void onRPMLimitReady(int rpmLimit);
    void onRPMTableReady();
    void onROMImageReady();
    void onROMImageReadFailed();
    void onInterfaceReady();
    void onNotConnected();
    void onFeedbackModeChanged(c14cux_feedback_mode mode);
    void onFuelMapIndexChanged(unsigned int fuelMapId);
#ifdef ENABLE_FORCE_OPEN_LOOP
    void onForceOpenLoopStateReceived(bool forceOpen);
#endif

signals:
    void requestToStartPolling();
    void requestFuelMapData(unsigned int fuelMapId);
    void requestROMImage();
    void requestThreadShutdown();
    void requestFuelPumpRun();

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *m_ui;

    #ifdef ENABLE_SIM_MODE
        QAction *m_simDialogAction;
        SimulationModeDialog *m_simDialog;
    #endif

    QTimer *m_fuelPumpRefreshTimer;
    QThread *m_cuxThread;
    CUXInterface *m_cux;
    OptionsDialog *m_options;
    IdleAirControlDialog *m_iacDialog;
    AboutBox *m_aboutBox;
    QMessageBox *m_pleaseWaitBox;
    HelpViewer *m_helpViewerDialog;

    Logger *m_logger;

    QGraphicsOpacityEffect *m_waterTempGaugeOpacity;
    QGraphicsOpacityEffect *m_fuelTempGaugeOpacity;
    QGraphicsOpacityEffect *m_speedometerOpacity;
    QGraphicsOpacityEffect *m_revCounterOpacity;
    QGraphicsOpacityEffect *m_fuelMapOpacity;
    QGraphicsOpacityEffect *m_idleModeLedOpacity;
    QGraphicsOpacityEffect *m_fuelPumpLedOpacity;

    QMap<SampleType,bool> m_enabledSamples;

    static const float speedometerMaxMPH = 160.0;
    static const float speedometerMaxKPH = 240.0;

    QTableWidgetItem * m_lastHighlightedFuelMapCell[NUM_ACTIVE_FUEL_MAP_CELLS];
    bool m_fuelMapDataIsCurrent;

    QHash<SpeedUnits,QString> *m_speedUnitSuffix;
    QHash<TemperatureUnits,QString> *m_tempUnitSuffix;
    QHash<TemperatureUnits,QPair<int,int> > *m_tempRange;
    QHash<TemperatureUnits,QPair<int,int> > *m_tempLimits;

    void doConnect();
    void startLogging();
    void buildSpeedAndTempUnitTables();
    void setupWidgets();
    void populateFuelMapDisplay(QByteArray* data, unsigned int fuelMapMultiplier, unsigned int rowScaler);
    QColor getColorForFuelMapCell(unsigned char value);
    void highlightActiveFuelMapCells();
    void removeFuelMapCellHighlight();
    void sendROMImageRequest(QString prompt);
    void dimUnusedControls();    
    void setGearLabel(c14cux_gear gearReading);
    void setLambdaTrimIndicators(int lambdaTrimOdd, int lambdaTrimEven);
    void setLambdaWidgetsForFeedbackMode(c14cux_feedback_mode mode, bool coTrimEnabled, bool lambdaEnabled);

private slots:
    void onSaveROMImageSelected();
    void onROMReadCancelled();
    void onExitSelected();
    void onEditOptionsClicked();
    void onHelpContentsClicked();
    void onHelpAboutClicked();
    void onConnectClicked();
    void onDisconnectClicked();
    void onStartLogging();
    void onStopLogging();
    void onFuelPumpContinuous();
    void onIdleAirControlClicked();
    void onLambdaTrimButtonClicked(QAbstractButton *button);
    void onMAFReadingButtonClicked(QAbstractButton *button);
    void onThrottleTypeButtonClicked(QAbstractButton *button);
#ifdef ENABLE_SIM_MODE
    void onSimDialogClicked();
#endif
};

#endif // MAINWINDOW_H


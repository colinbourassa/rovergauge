#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
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
    void onFuelMapDataReady(int fuelMapId);
    void onTuneRevisionReady(int tuneRevisionNum);
    void onRPMLimitReady(int rpmLimit);
    void onPROMImageReady();
    void onPROMImageReadFailed();
    void onInterfaceReady();
    void onNotConnected();

signals:
    void requestToStartPolling();
    void requestFuelMapData(int fuelMapId);
    void requestPROMImage();
    void requestThreadShutdown();
    void requestFuelPumpRun();

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *m_ui;

    QMenu *m_fileMenu;
    QAction *m_savePROMImageAction;
    QAction *m_exitAction;
    QMenu *m_optionsMenu;
    QAction *m_editOptionsAction;
    QAction *m_showFaultsAction;
    QAction *m_showIdleAirControlDialog;
    QMenu *m_helpMenu;
    QAction *m_helpAction;
    QAction *m_aboutAction;
    #ifdef ENABLE_SIM_MODE
        QAction *m_simDialogAction;
        SimulationModeDialog *m_simDialog;
    #endif

    QVBoxLayout *m_layout;

    QHBoxLayout *m_connectionButtonLayout;
    QHBoxLayout *m_commsLedLayout;

    QHBoxLayout *m_gaugesLayout;
    QHBoxLayout *m_aboveGaugesRow;
    QHBoxLayout *m_belowGaugesRow;

    QVBoxLayout *m_speedoLayout;
    QVBoxLayout *m_revCounterLayout;
    QVBoxLayout *m_waterTempLayout;
    QVBoxLayout *m_fuelTempLayout;
    QGridLayout *m_belowGaugesLeft;
    QGridLayout *m_belowGaugesRight;

    QPushButton *m_connectButton;
    QPushButton *m_disconnectButton;

    QLabel *m_tuneRevNumberLabel;

    QLabel *m_milLabel;
    QLedIndicator *m_milLed;

    QLabel *m_commsLedLabel;
    QLedIndicator *m_commsGoodLed;
    QLedIndicator *m_commsBadLed;

    ManoMeter *m_speedo;
    ManoMeter *m_revCounter;
    ManoMeter *m_waterTempGauge;
    ManoMeter *m_fuelTempGauge;

    QLabel *m_waterTempLabel;
    QLabel *m_fuelTempLabel;

    QProgressBar *m_throttleBar;
    QLabel *m_throttleLabel;

    QProgressBar *m_mafReadingBar;
    QLabel *m_mafReadingLabel;

    QProgressBar *m_idleBypassPosBar;
    QLabel *m_idleBypassLabel;

    QLabel *m_mafReadingTypeLabel;
    QButtonGroup *m_mafReadingButtonGroup;
    QRadioButton *m_mafReadingDirectButton;
    QRadioButton *m_mafReadingLinearButton;

    QLabel *m_lambdaTrimTypeLabel;
    QButtonGroup *m_lambdaTrimButtonGroup;
    QLabel *m_lambdaTrimLowLimitLabel;
    QLabel *m_lambdaTrimHighLimitLabel;
    QRadioButton *m_lambdaTrimShortButton;
    QRadioButton *m_lambdaTrimLongButton;

    QLabel *m_throttleTypeLabel;
    QButtonGroup *m_throttleTypeButtonGroup;
    QRadioButton *m_throttleTypeAbsoluteButton;
    QRadioButton *m_throttleTypeCorrectedButton;

    FuelTrimBar *m_leftFuelTrimBar;
    QLabel *m_leftFuelTrimBarLabel;
    QLabel *m_leftFuelTrimLabel;

    FuelTrimBar *m_rightFuelTrimBar;
    QLabel *m_rightFuelTrimBarLabel;
    QLabel *m_rightFuelTrimLabel;

    QHBoxLayout *m_idleSpeedLayout;
    QLabel *m_targetIdleLabel;
    QLabel *m_targetIdle;
    QLedIndicator *m_idleModeLed;

    QLabel *m_gearLabel;
    QLabel *m_gear;
    QLabel *m_voltageLabel;
    QLabel *m_voltage;

    QLabel *m_fuelMapIndexLabel;
    QLabel *m_fuelMapFactorLabel;
    QTableWidget *m_fuelMapDisplay;
    QLabel *m_fuelPumpRelayStateLabel;
    QLedIndicator *m_fuelPumpRelayStateLed;
    QPushButton *m_fuelPumpOneshotButton;
    QPushButton *m_fuelPumpContinuousButton;
    QTimer *m_fuelPumpRefreshTimer;

    QLabel *m_logFileNameLabel;
    QLineEdit *m_logFileNameBox;
    QPushButton *m_startLoggingButton;
    QPushButton *m_stopLoggingButton;

    QThread *m_cuxThread;
    CUXInterface *m_cux;
    OptionsDialog *m_options;
    IdleAirControlDialog *m_iacDialog;
    AboutBox *m_aboutBox;
    QMessageBox *m_pleaseWaitBox;
    HelpViewer *m_helpViewerDialog;

    Logger *m_logger;

    QFrame *m_horizontalLineA;
    QFrame *m_horizontalLineB;
    QFrame *m_horizontalLineC;
    QFrame *m_verticalLineA;
    QFrame *m_verticalLineB;
    QFrame *m_verticalLineC;

    QGraphicsOpacityEffect *m_waterTempGaugeOpacity;
    QGraphicsOpacityEffect *m_fuelTempGaugeOpacity;
    QGraphicsOpacityEffect *m_speedometerOpacity;
    QGraphicsOpacityEffect *m_revCounterOpacity;
    QGraphicsOpacityEffect *m_fuelMapOpacity;

    QGraphicsOpacityEffect *m_idleModeLedOpacity;
    QGraphicsOpacityEffect *m_fuelPumpLedOpacity;

    QHash<SampleType,bool> m_enabledSamples;

    int m_currentFuelMapIndex;
    int m_currentFuelMapRow;
    int m_currentFuelMapCol;

    int m_widthPixels;
    int m_heightPixels;

    QHash<SpeedUnits,QString> *m_speedUnitSuffix;
    QHash<TemperatureUnits,QString> *m_tempUnitSuffix;
    QHash<TemperatureUnits,QPair<int,int> > *m_tempRange;
    QHash<TemperatureUnits,QPair<int,int> > *m_tempLimits;

    void setupLayout();
    void createWidgets();
    void placeWidgets();
    void setupWidgets();

    void populateFuelMapDisplay(QByteArray* data);
    QColor getColorForFuelMapCell(unsigned char value);
    void highlightActiveFuelMapCell();
    void sendPROMImageRequest(QString prompt);
    void dimUnusedControls();

    void setGearLabel(Comm14CUXGear gearReading);
    void setLambdaTrimIndicators(int leftLambdaTrim, int rightLambdaTrim);

private slots:
    void onSavePROMImageSelected();
    void onPROMReadCancelled();
    void onExitSelected();
    void onEditOptionsClicked();
    void onHelpContentsClicked();
    void onHelpAboutClicked();
    void onConnectClicked();
    void onDisconnectClicked();
    void onStartLogging();
    void onStopLogging();
    void onFuelPumpOneshot();
    void onFuelPumpContinuous();
    void onFuelPumpRefreshTimer();
    void onIdleAirControlClicked();
    void onLambdaTrimButtonClicked(int id);
    void onMAFReadingButtonClicked(int id);
    void onThrottleTypeButtonClicked(int id);
#ifdef ENABLE_SIM_MODE
    void onSimDialogClicked();
#endif
};

#endif // MAINWINDOW_H


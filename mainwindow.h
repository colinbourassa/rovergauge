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
#include <QSlider>
#include <analogwidgets/manometer.h>
#include <qledindicator/qledindicator.h>
#include "optionsdialog.h"
#include "idleaircontroldialog.h"
#include "cuxinterface.h"
#include "aboutbox.h"
#include "logger.h"
#include "fueltrimbar.h"

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
    Ui::MainWindow *ui;

    QMenu *fileMenu;
    QAction *savePROMImageAction;
    QAction *exitAction;
    QMenu *optionsMenu;
    QAction *editOptionsAction;
    QAction *showFaultsAction;
    QAction *showIdleAirControlDialog;
    QMenu *helpMenu;
    QAction *aboutAction;

    QVBoxLayout *layout;

    QHBoxLayout *connectionButtonLayout;
    QHBoxLayout *commsLedLayout;

    QHBoxLayout *gaugesLayout;
    QHBoxLayout *aboveGaugesRow;
    QHBoxLayout *belowGaugesRow;

    QVBoxLayout *speedoLayout;
    QVBoxLayout *revCounterLayout;
    QVBoxLayout *waterTempLayout;
    QVBoxLayout *fuelTempLayout;
    QGridLayout *belowGaugesLeft;
    QGridLayout *belowGaugesRight;

    QPushButton *connectButton;
    QPushButton *disconnectButton;

    QLabel *commsLedLabel;
    QLedIndicator *commsGoodLed;
    QLedIndicator *commsBadLed;

    ManoMeter *speedo;
    ManoMeter *revCounter;
    ManoMeter *waterTempGauge;
    ManoMeter *fuelTempGauge;

    QLabel *waterTempLabel;
    QLabel *fuelTempLabel;

    QProgressBar *throttleBar;
    QLabel *throttleLabel;

    QProgressBar *mafReadingBar;
    QLabel *mafReadingLabel;

    QProgressBar *idleBypassPosBar;
    QLabel *idleBypassLabel;

    FuelTrimBar *leftFuelTrimBar;
    QLabel *leftFuelTrimBarLabel;
    QLabel *leftFuelTrimLabel;

    FuelTrimBar *rightFuelTrimBar;
    QLabel *rightFuelTrimBarLabel;
    QLabel *rightFuelTrimLabel;

    QLabel *targetIdleLabel;
    QLabel *targetIdle;

    QLabel *gearLabel;
    QLabel *gear;
    QLabel *voltageLabel;
    QLabel *voltage;

    QLabel *fuelMapIndexLabel;
    QLabel *fuelMapFactorLabel;
    QTableWidget *fuelMapDisplay;
    QLabel *fuelPumpRelayStateLabel;
    QLedIndicator *fuelPumpRelayStateLed;
    QPushButton *fuelPumpOneshotButton;
    QPushButton *fuelPumpContinuousButton;
    QTimer *fuelPumpRefreshTimer;

    QLabel *logFileNameLabel;
    QLineEdit *logFileNameBox;
    QPushButton *startLoggingButton;
    QPushButton *stopLoggingButton;

    QThread *cuxThread;
    CUXInterface *cux;
    OptionsDialog *options;
    IdleAirControlDialog *iacDialog;
    AboutBox *aboutBox;
    QMessageBox *pleaseWaitBox;

    Logger *logger;

    QFrame *horizontalLineA;
    QFrame *horizontalLineB;
    QFrame *horizontalLineC;
    QFrame *verticalLineA;

    int currentFuelMapIndex;
    int currentFuelMapRow;
    int currentFuelMapCol;

    void setupLayout();
    void createWidgets();
    void placeWidgets();
    void setupWidgets();

    QHash<SpeedUnits,QString> *speedUnitSuffix;
    QHash<TemperatureUnits,QString> *tempUnitSuffix;
    QHash<TemperatureUnits,QPair<int,int> > *tempRange;
    QHash<TemperatureUnits,QPair<int,int> > *tempLimits;

    void populateFuelMapDisplay(QByteArray* data);
    QColor getColorForFuelMapCell(unsigned char value);
    void highlightActiveFuelMapCell();

private slots:
    void onSavePROMImageSelected();
    void onPROMReadCancelled();
    void onExitSelected();
    void onEditOptionsClicked();
    void onHelpAboutClicked();
    void onConnectClicked();
    void onDisconnectClicked();
    void onStartLogging();
    void onStopLogging();
    void onFuelPumpOneshot();
    void onFuelPumpContinuous();
    void onFuelPumpRefreshTimer();
    void onIdleAirControlClicked();
};

#endif // MAINWINDOW_H


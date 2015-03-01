#include <QApplication>
#include "mainwindow.h"

// TODO: Temporary nonstandard baud rate experiment.
bool g_doubleBaud;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    g_doubleBaud = ((argc > 1) && (strcmp(argv[1], "-d") == 0));
    MainWindow w;
    w.show();
    return a.exec();
}


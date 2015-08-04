#include <QApplication>
#include "mainwindow.h"

bool g_doubleBaud;
bool g_autoconnect;
bool g_autolog;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    g_doubleBaud = false;
    g_autoconnect = false;
    g_autolog = false;

    for (unsigned int idx = 1; idx < argc; idx += 1)
    {
      if (strcmp(argv[idx], "-d") == 0)
      {
        g_doubleBaud = true;
      }
      else if (strcmp(argv[idx], "-a") == 0)
      {
        g_autoconnect = true;
      }
      else if (strcmp(argv[idx], "-l") == 0)
      {
        g_autolog = true;
      }
    }

    MainWindow w;
    w.show();
    return a.exec();
}


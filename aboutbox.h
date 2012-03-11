#ifndef ABOUTBOX_H
#define ABOUTBOX_H

#include <QDialog>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QStyle>
#include <QString>
#include "comm14cux.h"

/**
 * The "About" dialog that shows information about the program.
 */
class AboutBox : public QDialog
{
    Q_OBJECT

public:
    AboutBox(QStyle *parentStyle, QString title, Comm14CUXVersion version);

private:
    void setupWidgets();
    QString makeVersionString(int maj, int min, int patch);

    QStyle *style;
    QGridLayout *grid;

    Comm14CUXVersion ver;

    const QString urlString;
    const QString urlLibString;
    const QString aboutString;
    QLabel *iconLabel;
    QLabel *info;
    QLabel *name;
    QLabel *infoLib;
    QLabel *url;
    QLabel *urlLib;
    QPushButton *ok;
};

#endif // ABOUTBOX_H


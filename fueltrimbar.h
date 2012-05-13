#ifndef FUELTRIMBAR_H
#define FUELTRIMBAR_H

#include <QProgressBar>

class FuelTrimBar : public QProgressBar
{
    Q_OBJECT
public:
    explicit FuelTrimBar(QWidget *parent = 0);
    void paintEvent(QPaintEvent *e);

signals:

public slots:

private:
    const int minimumVal;
    const int maximumVal;
};

#endif // FUELTRIMBAR_H

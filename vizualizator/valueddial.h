#ifndef VALUEDDIAL_H
#define VALUEDDIAL_H

#include <QDial>
#include <QPainter>
class ValuedDial : public QDial
{
    Q_OBJECT
public:
    explicit ValuedDial(QWidget *parent = 0);

private:
    void paintEvent(QPaintEvent *pe);
signals:

public slots:

};

#endif // VALUEDDIAL_H

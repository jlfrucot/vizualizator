#include "valueddial.h"

ValuedDial::ValuedDial(QWidget *parent) :
    QDial(parent)
{
}

void ValuedDial::paintEvent(QPaintEvent *pe)
{
    QDial::paintEvent(pe);
    QPainter painter(this);
        QRect rect = geometry();
        int x = rect.width()/3;
        int y = rect.height()/2;
        painter.setPen(QPen(Qt::red));
        int angle = (360-value())%360;
        painter.drawText(QPoint(x,y),QString::number(angle)+"°");
}

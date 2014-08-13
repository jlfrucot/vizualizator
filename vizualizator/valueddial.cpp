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
        /* Le calcul ci-desous permet d'avoir les valeurs dans le bon sens (antihoraire)
         * et 0 à la place de 360
         */
        painter.drawText(QPoint(x,y),QString::number((360-value())%360)+"°");
}

#ifndef VIZUALIZATORWIDGET_H
#define VIZUALIZATORWIDGET_H

#include <QWidget>

namespace Ui {
class VizualizatorWidget;
}

class VizualizatorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VizualizatorWidget(QWidget *parent = 0);
    ~VizualizatorWidget();

private:
    Ui::VizualizatorWidget *ui;
};

#endif // VIZUALIZATORWIDGET_H

#include "vizualizatorwidget.h"
#include "ui_vizualizatorwidget.h"

VizualizatorWidget::VizualizatorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VizualizatorWidget)
{
    ui->setupUi(this);
}

VizualizatorWidget::~VizualizatorWidget()
{
    delete ui;
}

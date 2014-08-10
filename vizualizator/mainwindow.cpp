#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /* On place la liste des QActions dans le menu Caméras de la mainwindow */
    ui->menuCamera->addActions(ui->widgetVizu->vizualizatorWidgetGetVideoDevicesGroup()->actions());
}

MainWindow::~MainWindow()
{
    delete ui;


}

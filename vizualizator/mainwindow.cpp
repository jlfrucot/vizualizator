/**
  * Vizualizator
  *
  * @warning aucun traitement d'erreur n'est pour l'instant implémenté
  * @see https://github.com/jlfrucot/vizualizator
  * @author 2014 Jean-Louis Frucot <frucot.jeanlouis@free.fr>
  * @see The GNU Public License (GPL)
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful, but
  * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  * for more details.
  *
  * You should have received a copy of the GNU General Public License along
  * with this program; if not, write to the Free Software Foundation, Inc.,
  * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
  */


#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /* On place la liste des QActions dans le menu Caméras de la mainwindow si on veut */
    ui->menuCamera->addActions(ui->widgetVizu->vizualizatorWidgetGetVideoDevicesGroup()->actions());
    /* Pour le fun
     * Les deux lignes suivantes ne sont pas nécessaires c'est ppour la démo ;-)
     */
    ui->menuBar->setVisible(false);
    ui->dockWidget->setWidget(ui->widgetVizu->VizualizatorWidgetGetToolBox());
}

MainWindow::~MainWindow()
{
    delete ui;


}

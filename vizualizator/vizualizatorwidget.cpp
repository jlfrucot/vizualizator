/**
  * VizualizatorWidget
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


#include "vizualizatorwidget.h"
#include "ui_vizualizatorwidget.h"


VizualizatorWidget::VizualizatorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VizualizatorWidget),
    m_scene(0),
    m_sceneImage(0),
    m_camera(0),
    m_imageCapture(0),
    m_imageItem(0),
    m_currentImage(0),
    m_viewfinder(0),
    m_mediaRecorder(0),
    m_isCapturingImage(false),
    m_applicationExiting(false)
{
    m_localDebug = true;
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__<<"Constructor";
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    ui->tbToolPanel->setCurrentWidget(ui->pageCamera);
    ui->tabWidget->setCurrentWidget(ui->tabCamera1);
    ui->gLContrastBrightness->setAlignment(ui->vSBrightness, Qt::AlignHCenter);
    ui->gLContrastBrightness->setAlignment(ui->vSContrast, Qt::AlignHCenter);
    m_fullscreenLabels.clear();

    m_scene = new QGraphicsScene();
    ui->gvCameraView->setScene(m_scene);
    ui->gvCameraView->setSceneRect(0,0, ui->gvCameraView->width(), ui->gvCameraView->height());

    m_sceneImage = new QGraphicsScene();
    ui->gvImage->setScene(m_sceneImage);
    ui->gvImage->setSceneRect(0,0, ui->gvImage->width(), ui->gvImage->height());
    /* On va travailler dans une QGraphicsView */
    m_viewfinder = new QGraphicsVideoItem();
    m_scene->addItem(m_viewfinder);
    m_viewfinder->setSize(ui->gvCameraView->size());
    m_viewfinder->setTransformOriginPoint(m_viewfinder->boundingRect().center());

    m_transformRotation.reset(); // Matrice identité (ne fait rien)
    m_transformXaxisMirror.reset();
    m_transformYaxisMirror.reset();

    /* On fait la liste des Camera devices:
     * On les place aussi dans un QActionGroup
     * que l'on pourra utiliser pour mettre dans un menu par exemple
     * voir (vizualizatorWidgetGetVideoDevicesGroup()
     * Et on affecte la première au widget
     */
    setCamera(vizualizatorGetCameras());

}

VizualizatorWidget::~VizualizatorWidget()
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__<<"Destructeur";
    if(m_currentImage)
    {
        delete m_currentImage;
    }
    if (m_camera)
    {
        m_camera->stop();
        delete m_camera;
    }
    delete ui;
}

QByteArray VizualizatorWidget::vizualizatorGetCameras()
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    QByteArray cameraDevice;
    m_videoDevicesGroup = new QActionGroup(this);
    m_videoDevicesGroup->setExclusive(true);
    /* On crée les QAction correspondant à chaque caméra */
    foreach(const QByteArray &deviceName, QCamera::availableDevices())
    {
        QString description = m_camera->deviceDescription(deviceName);
        QAction *videoDeviceAction = new QAction(description, m_videoDevicesGroup);
        videoDeviceAction->setCheckable(true);
        videoDeviceAction->setData(QVariant(deviceName));
        /* La caméra par défaut */
        if (cameraDevice.isEmpty())
        {
            cameraDevice = deviceName;
            videoDeviceAction->setChecked(true);
        }
        // On va remplir la combobox de sélection de camera avec la description et le pointeur vers la QAction
        ui->cBoxSelectCam->insertItem(ui->cBoxSelectCam->count(),description, VariantPtr<QAction>::asQVariant(videoDeviceAction));
    }
    /* Pour changer de caméra */
    connect(m_videoDevicesGroup, SIGNAL(triggered(QAction*)), SLOT(updateCameraDevice(QAction*)));

    return cameraDevice; // La camera par défaut
}

QWidget *VizualizatorWidget::VizualizatorWidgetGetToolBox()
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    return ui->widgetToolPanel;
}

void VizualizatorWidget::setCamera(const QByteArray &cameraDevice)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    /* On détruit les éventuels objects devenus obsoletes */
    m_imageCapture->deleteLater();
    m_mediaRecorder->deleteLater();
    m_camera->deleteLater();

    /* On crée ce dont on a besoin */

    /* Une camera */
    if (cameraDevice.isEmpty())
    {
        m_camera = new QCamera;
    }
    else
    {
        m_camera = new QCamera(cameraDevice);
    }
    connect(m_camera, SIGNAL(stateChanged(QCamera::State)), this, SLOT(updateCameraState(QCamera::State)));
    connect(m_camera, SIGNAL(error(QCamera::Error)), this, SLOT(displayCameraError()));

    /* Un recorder pour enregistrer la vidéo */
    m_mediaRecorder = new QMediaRecorder(m_camera);
    m_mediaRecorder->setMetaData(QMediaMetaData::Title, QVariant(QLatin1String("Test Title")));
    connect(m_mediaRecorder, SIGNAL(stateChanged(QMediaRecorder::State)), this, SLOT(updateRecorderState(QMediaRecorder::State)));
    connect(m_mediaRecorder, SIGNAL(durationChanged(qint64)), this, SLOT(updateRecordTime()));
    connect(m_mediaRecorder, SIGNAL(error(QMediaRecorder::Error)), this, SLOT(displayRecorderError()));

    /* Un "captureur" d'images
     * Avec beaucoup de connect : on fera le ménage plus tard
     */
    m_imageCapture = new QCameraImageCapture(m_camera);
    m_imageCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
    connect(m_imageCapture, SIGNAL(readyForCaptureChanged(bool)), this, SLOT(slotReadyForCapture(bool)), Qt::UniqueConnection);
    connect(m_imageCapture, SIGNAL(imageExposed(int)), this, SLOT(slotImageExposed(int)), Qt::UniqueConnection);
    connect(m_imageCapture, SIGNAL(imageCaptured(int,QImage)), this, SLOT(processCapturedImage(int,QImage)));
    connect(m_imageCapture, SIGNAL(imageSaved(int,QString)), this, SLOT(imageSaved(int,QString)));
    connect(m_imageCapture, SIGNAL(error(int,QCameraImageCapture::Error,QString)),
            this,           SLOT(displayCaptureError(int,QCameraImageCapture::Error,QString)));

    /* On affiche la caméra dans le bon widget */
    m_camera->setViewfinder(m_viewfinder);
    connect(m_camera, SIGNAL(lockStatusChanged(QCamera::LockStatus, QCamera::LockChangeReason)),
            this, SLOT(updateLockStatus(QCamera::LockStatus, QCamera::LockChangeReason)));

//    m_camera->searchAndLock();
    updateCameraState(m_camera->state());
    updateLockStatus(m_camera->lockStatus(), QCamera::UserRequest);
    updateRecorderState(m_mediaRecorder->state());
    updateCaptureMode();

    m_camera->start(); // Et c'est parti
}
void VizualizatorWidget::updateCameraDevice(QAction *action)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__<<action->text();
    setCamera(action->data().toByteArray());
    // On maximise la taille de la caméra dans la QGraphicsView
    ui->gvCameraView->fitInView(m_viewfinder, Qt::KeepAspectRatio);
}
void VizualizatorWidget::updateRecordTime()
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    QString str = QString("Recorded %1 sec").arg(m_mediaRecorder->duration()/1000);
}

void VizualizatorWidget::showResizedImage()
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    if(m_currentImage)
    {
        m_currentImage->setAngleRotation(ui->dialOrientation->value());
        m_currentImage->setFlipXaxis(ui->btnXaxisMirror->isChecked());
        m_currentImage->setFlipYaxis(ui->btnYaxisMirror->isChecked());
        ui->gvImage->setSceneRect(ui->gvImage->rect());
        if(ui->cbNativeImage->isChecked())
        {
            QImage scaledImage = m_currentImage->getOriginalImage().scaled(ui->gvImage->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
            m_imageItem->setPixmap(QPixmap::fromImage(scaledImage));
        }
        else
        {
            QImage scaledImage = m_currentImage->getModifiedImage()
                    .scaled(ui->gvImage->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
            m_imageItem->setPixmap(QPixmap::fromImage(scaledImage));
        }
        m_imageItem->setPos((ui->gvImage->width()-m_imageItem->boundingRect().width())/2,
                            (ui->gvImage->height()-m_imageItem->boundingRect().height())/2);
        slotUpdateThumbnailItem();
    }
}

void VizualizatorWidget::processCapturedImage(int requestId, const QImage& img)
{

    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    Q_UNUSED(requestId);
    ui->tabGallery->setVisible(true);
    ui->tabWidget->setCurrentWidget(ui->tabGallery);
    ui->tbToolPanel->setCurrentWidget(ui->pageImagesSettings);
    if(!m_imageItem)
    {
        m_imageItem = new QGraphicsPixmapItem();
        m_sceneImage->addItem(m_imageItem);
    }
    /* On va garder les m_image pour voir les réafficher à la demande */
//    if(m_image)
//    {
//        delete m_image;
//    }
    m_currentImage = new VizualizatorImage(img);
    /* Pour laisser le temps au widget de prendre sa taille ??? */
    QTimer::singleShot(100,this,SLOT(showResizedImage()));

    /* On crée et affiche un thumbnail */
    QListWidgetItem *item = new QListWidgetItem();
    /* On garde le pointeur de m_image avec l'item */
    item->setData(ImagePointer,VariantPtr<VizualizatorImage>::asQVariant(m_currentImage));
    ui->lwGallery->insertItem(ui->lwGallery->count(),item);
    ui->lwGallery->setCurrentItem(item);
}

void VizualizatorWidget::configureCaptureSettings()
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    switch (m_camera->captureMode()) {
    case QCamera::CaptureStillImage:
        configureImageSettings();
        break;
    case QCamera::CaptureVideo:
        configureVideoSettings();
        break;
    default:
        break;
    }
}

void VizualizatorWidget::configureVideoSettings()
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    //    VideoSettings settingsDialog(mediaRecorder);

    //    settingsDialog.setAudioSettings(audioSettings);
    //    settingsDialog.setVideoSettings(videoSettings);
    //    settingsDialog.setFormat(videoContainerFormat);

    //    if (settingsDialog.exec()) {
    //        audioSettings = settingsDialog.audioSettings();
    //        videoSettings = settingsDialog.videoSettings();
    //        videoContainerFormat = settingsDialog.format();

    //        mediaRecorder->setEncodingSettings(
    //                    audioSettings,
    //                    videoSettings,
    //                    videoContainerFormat);
    //    }
}

void VizualizatorWidget::configureImageSettings()
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    //    ImageSettings settingsDialog(imageCapture);

    //    settingsDialog.setImageSettings(imageSettings);

    //    if (settingsDialog.exec()) {
    //        m_imageSettings = settingsDialog.imageSettings();
    //        m_imageCapture->setEncodingSettings(imageSettings);
    //    }
}

void VizualizatorWidget::record()
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    m_mediaRecorder->record();
    updateRecordTime();
}

void VizualizatorWidget::pause()
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    m_mediaRecorder->pause();
}

void VizualizatorWidget::stop()
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    m_mediaRecorder->stop();
}

void VizualizatorWidget::setMuted(bool muted)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    m_mediaRecorder->setMuted(muted);
}

void VizualizatorWidget::toggleLock()
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    switch (m_camera->lockStatus()) {
    case QCamera::Searching:
    case QCamera::Locked:
        m_camera->unlock();
        break;
    case QCamera::Unlocked:
        m_camera->searchAndLock();
    }
}

void VizualizatorWidget::updateLockStatus(QCamera::LockStatus status, QCamera::LockChangeReason reason)
{
    Q_UNUSED(reason)
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__<<status<<reason;
    switch (status)
    {
    case QCamera::Searching:
        if (m_localDebug) qDebug()<<"          Searching";
        break;
    case QCamera::Locked:
        if (m_localDebug) qDebug()<<"          Locked";
        break;
    case QCamera::Unlocked:
        if (m_localDebug) qDebug()<<"          Unlocked";
        break;
    }
}

void VizualizatorWidget::displayCaptureError(int id, const QCameraImageCapture::Error error, const QString &errorString)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    Q_UNUSED(id);
    Q_UNUSED(error);
    QMessageBox::warning(this, tr("Image Capture Error"), errorString);
    m_isCapturingImage = false;
}

void VizualizatorWidget::startCamera()
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    m_camera->start();
}

void VizualizatorWidget::stopCamera()
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    m_camera->stop();
}

void VizualizatorWidget::updateCaptureMode()
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;

    /* Pour l'instant on ne prend que des images */
    QCamera::CaptureModes captureMode = QCamera::CaptureStillImage;

    if (m_camera->isCaptureModeSupported(captureMode))
    {
        if (m_localDebug) qDebug()<<"                       captureMode supported;";
        m_camera->setCaptureMode(captureMode);
    }
}

void VizualizatorWidget::updateCameraState(QCamera::State state)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    switch (state) {
    case QCamera::ActiveState:
        //        ui->actionStartCamera->setEnabled(false);
        //        ui->actionStopCamera->setEnabled(true);
        //        ui->captureWidget->setEnabled(true);
        //        ui->actionSettings->setEnabled(true);
        break;
    case QCamera::UnloadedState:
    case QCamera::LoadedState:
        //        ui->actionStartCamera->setEnabled(true);
        //        ui->actionStopCamera->setEnabled(false);
        //        ui->captureWidget->setEnabled(false);
        //        ui->actionSettings->setEnabled(false);
        break;
    }
}

void VizualizatorWidget::updateRecorderState(QMediaRecorder::State state)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    //    switch (state) {
    //    case QMediaRecorder::StoppedState:
    //        ui->recordButton->setEnabled(true);
    //        ui->pauseButton->setEnabled(true);
    //        ui->stopButton->setEnabled(false);
    //        break;
    //    case QMediaRecorder::PausedState:
    //        ui->recordButton->setEnabled(true);
    //        ui->pauseButton->setEnabled(false);
    //        ui->stopButton->setEnabled(true);
    //        break;
    //    case QMediaRecorder::RecordingState:
    //        ui->recordButton->setEnabled(false);
    //        ui->pauseButton->setEnabled(true);
    //        ui->stopButton->setEnabled(true);
    //        break;
    //    }
}
void VizualizatorWidget::displayRecorderError()
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    QMessageBox::warning(this, tr("Capture error"), m_mediaRecorder->errorString());
}

void VizualizatorWidget::displayCameraError()
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    QMessageBox::warning(this, tr("Camera error"), m_camera->errorString());
}

void VizualizatorWidget::imageSaved(int id, const QString &fileName)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__<<fileName;
    Q_UNUSED(id);
    Q_UNUSED(fileName);

    m_isCapturingImage = false;
    

    if (m_applicationExiting)
        close();
    
}

void VizualizatorWidget::closeEvent(QCloseEvent *event)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    if (m_isCapturingImage)
    {
        setEnabled(false);
        m_applicationExiting = true;
        event->ignore();
    }
    else
    {
        event->accept();
    }
}


void VizualizatorWidget::on_btnTakePicture_clicked()
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    m_isCapturingImage = true;
    m_imageCapture->capture();
}

void VizualizatorWidget::slotReadyForCapture(bool ready)
{

    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__<< ready;
    ui->btnTakePicture->setEnabled(ready);
}

void VizualizatorWidget::slotImageExposed(int id)
{

    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__<< id;
}

void VizualizatorWidget::on_dialOrientation_valueChanged(int value)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    if(value<5 || value > 354)
    {
        value = 0;
        ui->rbRotate0deg->setChecked(true);
    }
    else if(value > 84 && value <91)
    {
        value = 90;
        ui->rbRotate90deg->setChecked(true);
    }
    else if(value > 174 && value <186)
    {
        value = 180;
        ui->rbRotate180deg->setChecked(true);
    }
    else if(value > 264 && value <276)
    {
        value = 270;
        ui->rbRotate270deg->setChecked(true);
    }
    ui->dialOrientation->setValue(value);
    m_transformRotation.reset();
    /* Les deux translate sont là pour que le widget tourne autour de son centre */
    m_transformRotation.translate(m_viewfinder->boundingRect().center().x(),m_viewfinder->boundingRect().center().y());
    m_transformRotation.rotate(value);
    m_transformRotation.translate(-m_viewfinder->boundingRect().center().x(),-m_viewfinder->boundingRect().center().y());


    // Et on affiche soit la caméra, soit l'image
    if(ui->tabWidget->currentWidget() == ui->tabCamera1)
    {
        updateViewfinderTransformations();
    }
    else if(ui->tabWidget->currentWidget() == ui->tabGallery)
    {
        m_currentImage->setAngleRotation(value);
        showResizedImage();
    }
}

void VizualizatorWidget::on_rbRotate0deg_clicked(bool checked)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__<<checked;
    if(checked)
    {
        ui->dialOrientation->setValue(0); // C'est le valuedDial qui se charge de la rotation
    }
}

void VizualizatorWidget::on_rbRotate90deg_clicked(bool checked)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__<<checked;
    if(checked)
    {
        ui->dialOrientation->setValue(90); // C'est le valuedDial qui se charge de la rotation
    }
}

void VizualizatorWidget::on_rbRotate180deg_clicked(bool checked)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__<<checked;
    if(checked)
    {
        ui->dialOrientation->setValue(180); // C'est le valuedDial qui se charge de la rotation
    }
}

void VizualizatorWidget::on_rbRotate270deg_clicked(bool checked)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__<<checked;
    if(checked)
    {
        ui->dialOrientation->setValue(270); // C'est le valuedDial qui se charge de la rotation
    }
}

void VizualizatorWidget::on_btnYaxisMirror_clicked(bool checked)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__<<checked;
    m_transformYaxisMirror.reset();
    if(checked)
    {
        m_transformYaxisMirror.rotate(180, Qt::YAxis);
        m_transformYaxisMirror.translate(-m_viewfinder->boundingRect().width(), 0);
    }
    if(ui->tabWidget->currentWidget() == ui->tabCamera1)
    {
        updateViewfinderTransformations();
    }
    else if(ui->tabWidget->currentWidget() == ui->tabGallery)
    {
        m_currentImage->setFlipYaxis(checked);
        showResizedImage();
    }
}

void VizualizatorWidget::on_btnXaxisMirror_clicked(bool checked)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__<<checked;
    m_transformXaxisMirror.reset();
    if(checked)
    {
        m_transformXaxisMirror.rotate(180, Qt::XAxis);
        m_transformXaxisMirror.translate(0, -m_viewfinder->boundingRect().height());
    }

    if(ui->tabWidget->currentWidget() == ui->tabCamera1)
    {
        updateViewfinderTransformations();
    }
    else if(ui->tabWidget->currentWidget() == ui->tabGallery)
    {
        m_currentImage->setFlipXaxis(checked);
        showResizedImage();
    }
}

void VizualizatorWidget::updateViewfinderTransformations()
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    m_viewfinder->setTransform(m_transformRotation, false);
    m_viewfinder->setTransform(m_transformYaxisMirror, true);
    m_viewfinder->setTransform(m_transformXaxisMirror, true);
    ui->gvCameraView->fitInView(m_viewfinder, Qt::KeepAspectRatio);
}


void VizualizatorWidget::resizeEvent(QResizeEvent *)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    ui->gvCameraView->fitInView(m_viewfinder,Qt::KeepAspectRatio);
    ui->gvImage->setSceneRect(0,0, ui->gvImage->width(), ui->gvImage->height());
    if(m_imageItem)
    {
        showResizedImage();
    }
}

void VizualizatorWidget::showEvent(QShowEvent *)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    ui->gvCameraView->fitInView(m_viewfinder,Qt::KeepAspectRatio);

    ui->gvImage->setSceneRect(0,0, ui->gvImage->width(), ui->gvImage->height());
    if(m_imageItem)
    {
        showResizedImage();
    }
}

void VizualizatorWidget::on_cbNativeImage_clicked(bool checked)
{
    Q_UNUSED(checked)
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    showResizedImage();
}

void VizualizatorWidget::on_btnFullScreenImage_clicked()
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    QImage scaledImage;
    if(ui->cbNativeImage->isChecked())
    {
        QSize size = QApplication::desktop()->availableGeometry().size();
        scaledImage = m_currentImage->getOriginalImage()
                .scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    else
    {
        QSize size = QApplication::desktop()->availableGeometry().size();
        {
            scaledImage = m_currentImage->getModifiedImage()
                    .scaled(size,Qt::KeepAspectRatio,Qt::SmoothTransformation);
        }        
        showFullScreen(scaledImage);
    }
}

void VizualizatorWidget::showFullScreen(QImage scaledImage)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    QLabel *label = new QLabel();
    label->setAlignment(Qt::AlignCenter);
    label->setPixmap(QPixmap::fromImage(scaledImage));
    m_fullscreenLabels.append(label);
    label->showMaximized();
}

void VizualizatorWidget::hideEvent(QHideEvent *)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    foreach (QLabel *label, m_fullscreenLabels)
    {
       if (m_localDebug)  qDebug()<<"FullScreen Labels Destructor";
        label->close();
        label->deleteLater();
    }
    m_fullscreenLabels.clear();
}



void VizualizatorWidget::on_tabWidget_currentChanged(int index)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    switch(index)
    {
    case 0: // TAb Camera
        ui->tbToolPanel->setCurrentWidget(ui->pageCamera);
        break;
    case 1: // tab Gallery
        ui->tbToolPanel->setCurrentWidget(ui->pageImagesSettings);

        showResizedImage();
        break;
    default:
        // On ne fait rien
        break;
    }
}

void VizualizatorWidget::on_tbToolPanel_currentChanged(int index)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    switch(index)
    {
    case 0: // TAb Camera
        ui->tabWidget->setCurrentWidget(ui->tabCamera1);
        break;
    case 1: // tab Gallery
        ui->tabWidget->setCurrentWidget(ui->tabGallery);
        break;
    case 3:
        // Pas implémenté
        break;
    default:
        // On ne fait rien
        break;
    }
}

void VizualizatorWidget::restoreUiFromItem(QListWidgetItem *item)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    ui->dialOrientation->setValue(item->data(Rotation).toInt());
    ui->btnXaxisMirror ->setChecked(item->data(FlipXaxis).toBool());
    ui->btnYaxisMirror ->setChecked(item->data(FlipYaxis).toBool());
    ui->cbNativeImage  ->setChecked(false);     // Pourquoi pas ?
}

void VizualizatorWidget::on_lwGallery_itemClicked(QListWidgetItem *item)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    /* On va réafficher l'image correspondant à l'item sélectionné */
    m_currentImage = VariantPtr<VizualizatorImage>::asPtr(item->data(ImagePointer));
    restoreUiFromItem(item);
    showResizedImage();
}

void VizualizatorWidget::slotUpdateThumbnailItem()
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    QListWidgetItem *item = ui->lwGallery->currentItem();
    if(item)
    {
        item->setIcon(QPixmap::fromImage(m_currentImage->getThumbnail()));
    }
}


void VizualizatorWidget::on_cBoxSelectCam_currentIndexChanged(int index)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    // On va récupérer le pointeur de la QAction et la déclencher
    updateCameraDevice(VariantPtr<QAction>::asPtr(ui->cBoxSelectCam->itemData(index, Qt::UserRole)));
}

void VizualizatorWidget::on_vSBrightness_valueChanged(int value)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    m_currentImage->setBrightnessValue(value);
            showResizedImage();
}

void VizualizatorWidget::on_vSContrast_valueChanged(int value)
{
    if (m_localDebug) qDebug()<<__LINE__<<" ++++++++ " << __FUNCTION__;
    /* Le contraste variera entre 0 et 5 */
    m_currentImage->setContrastValue((qreal)value/500);
    showResizedImage();
}

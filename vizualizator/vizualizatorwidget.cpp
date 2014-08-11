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
    m_camera(0),
    m_imageCapture(0),
    m_viewfinder(0),
    m_mediaRecorder(0),
    m_isCapturingImage(false),
    m_applicationExiting(false)
{
    m_localDebug = true;
    ui->setupUi(this);
    m_transform.reset();
    m_scene = new QGraphicsScene();
    ui->gvCameraView->setScene(m_scene);
    ui->gvCameraView->setSceneRect(0,0, ui->gvCameraView->width(), ui->gvCameraView->height());


    m_viewfinder = new QGraphicsVideoItem();
    m_scene->addItem(m_viewfinder);
    m_viewfinder->setSize(ui->gvCameraView->size());
    m_viewfinder->setTransformOriginPoint(m_viewfinder->boundingRect().center());

    m_transform.rotate(180, Qt::YAxis);
    m_transform.translate(-m_viewfinder->boundingRect().width(), 0);
    m_viewfinder->setTransform(m_transform);
    /* On fait la liste des Camera devices:
     * On les place aussi dans un QActionGroup
     * que l'on pourra utiliser pour mettre dans un menu par exemple
     * voir (vizualizatorWidgetGetVideoDevicesGroup()
     */
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
            ui->lbCameraName->setText(videoDeviceAction->text());
        }

    }
    /* Pour changer de caméra */
    connect(m_videoDevicesGroup, SIGNAL(triggered(QAction*)), SLOT(updateCameraDevice(QAction*)));

    setCamera(cameraDevice);

}

VizualizatorWidget::~VizualizatorWidget()
{
    delete ui;
}

void VizualizatorWidget::setCamera(const QByteArray &cameraDevice)
{
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

    /* Un "captureur" d'images */
    m_imageCapture = new QCameraImageCapture(m_camera);
    connect(m_imageCapture, SIGNAL(readyForCaptureChanged(bool)), this, SLOT(slotReadyForCapture(bool)), Qt::UniqueConnection);
    connect(m_imageCapture, SIGNAL(imageExposed(int)), this, SLOT(slotImageExposed(int)), Qt::UniqueConnection);
//    connect(m_imageCapture, SIGNAL(readyForCaptureChanged(bool)), this, SLOT(readyForCapture(bool)));
    connect(m_imageCapture, SIGNAL(imageCaptured(int,QImage)), this, SLOT(processCapturedImage(int,QImage)));
    connect(m_imageCapture, SIGNAL(imageSaved(int,QString)), this, SLOT(imageSaved(int,QString)));
    connect(m_imageCapture, SIGNAL(error(int,QCameraImageCapture::Error,QString)), this,
            SLOT(displayCaptureError(int,QCameraImageCapture::Error,QString)));

    /* On affiche la caméra dans le bon widget */
    m_camera->setViewfinder(m_viewfinder);

    updateCameraState(m_camera->state());
    updateLockStatus(m_camera->lockStatus(), QCamera::UserRequest);
    updateRecorderState(m_mediaRecorder->state());

    connect(m_camera, SIGNAL(lockStatusChanged(QCamera::LockStatus, QCamera::LockChangeReason)),
            this, SLOT(updateLockStatus(QCamera::LockStatus, QCamera::LockChangeReason)));

    updateCaptureMode();
    m_camera->start();
}
void VizualizatorWidget::updateCameraDevice(QAction *action)
{
    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__<<action->text();
    setCamera(action->data().toByteArray());
    ui->lbCameraName->setText(action->text());
}
void VizualizatorWidget::updateRecordTime()
{
    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__;
    QString str = QString("Recorded %1 sec").arg(m_mediaRecorder->duration()/1000);
    //    ui->statusbar->showMessage(str);
}

void VizualizatorWidget::processCapturedImage(int requestId, const QImage& img)
{
    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__;
    Q_UNUSED(requestId);
    QImage scaledImage = img.scaled(m_viewfinder->size().toSize(),
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation);

    //    ui->lastImagePreviewLabel->setPixmap(QPixmap::fromImage(scaledImage));

    //    // Display captured image for 4 seconds.
    //    displayCapturedImage();
    //    QTimer::singleShot(4000, this, SLOT(displayViewfinder()));
}

void VizualizatorWidget::configureCaptureSettings()
{
    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__;
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
    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__;
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
    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__;
    //    ImageSettings settingsDialog(imageCapture);

    //    settingsDialog.setImageSettings(imageSettings);

    //    if (settingsDialog.exec()) {
    //        m_imageSettings = settingsDialog.imageSettings();
    //        m_imageCapture->setEncodingSettings(imageSettings);
    //    }
}

void VizualizatorWidget::record()
{
    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__;
    m_mediaRecorder->record();
    updateRecordTime();
}

void VizualizatorWidget::pause()
{
    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__;
    m_mediaRecorder->pause();
}

void VizualizatorWidget::stop()
{
    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__;
    m_mediaRecorder->stop();
}

void VizualizatorWidget::setMuted(bool muted)
{
    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__;
    m_mediaRecorder->setMuted(muted);
}

void VizualizatorWidget::toggleLock()
{
    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__;
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
    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__;
    QColor indicationColor = Qt::black;

    switch (status) {
    case QCamera::Searching:
        //        indicationColor = Qt::yellow;
        //        ui->statusbar->showMessage(tr("Focusing..."));
        //        ui->lockButton->setText(tr("Focusing..."));
        break;
    case QCamera::Locked:
        //        indicationColor = Qt::darkGreen;
        //        ui->lockButton->setText(tr("Unlock"));
        //        ui->statusbar->showMessage(tr("Focused"), 2000);
        break;
    case QCamera::Unlocked:
        //        indicationColor = reason == QCamera::LockFailed ? Qt::red : Qt::black;
        //        ui->lockButton->setText(tr("Focus"));
        //        if (reason == QCamera::LockFailed)
        //            ui->statusbar->showMessage(tr("Focus Failed"), 2000);
        //    }

        //    QPalette palette = ui->lockButton->palette();
        //    palette.setColor(QPalette::ButtonText, indicationColor);
        //    ui->lockButton->setPalette(palette);
        break;
    }
}

void VizualizatorWidget::takeImage()
{
    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__;
    m_isCapturingImage = true;
    m_imageCapture->capture();
}

void VizualizatorWidget::displayCaptureError(int id, const QCameraImageCapture::Error error, const QString &errorString)
{
    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__;
    Q_UNUSED(id);
    Q_UNUSED(error);
    QMessageBox::warning(this, tr("Image Capture Error"), errorString);
    m_isCapturingImage = false;
}

void VizualizatorWidget::startCamera()
{
    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__;
    m_camera->start();
}

void VizualizatorWidget::stopCamera()
{
    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__;
    m_camera->stop();
}

void VizualizatorWidget::updateCaptureMode()
{
    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__;
    //    int tabIndex = ui->captureWidget->currentIndex();
    //    QCamera::CaptureModes captureMode = tabIndex == 0 ? QCamera::CaptureStillImage : QCamera::CaptureVideo;

    /* Pour l'instant on ne prend que des images */
    QCamera::CaptureModes captureMode = QCamera::CaptureStillImage;

    if (m_camera->isCaptureModeSupported(captureMode))
    {
        m_camera->setCaptureMode(captureMode);
    }
}

void VizualizatorWidget::updateCameraState(QCamera::State state)
{
    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__;
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
    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__;
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
    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__;
    QMessageBox::warning(this, tr("Capture error"), m_mediaRecorder->errorString());
}

void VizualizatorWidget::displayCameraError()
{
    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__;
    QMessageBox::warning(this, tr("Camera error"), m_camera->errorString());
}
//void VizualizatorWidget::readyForCapture(bool ready)
//{
//    //    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__;
//    //    ui->btnTakePicture->setEnabled(ready);
//}

void VizualizatorWidget::imageSaved(int id, const QString &fileName)
{
    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__;
    Q_UNUSED(id);
    Q_UNUSED(fileName);

    m_isCapturingImage = false;
    
    if (m_applicationExiting)
        close();
    
}

void VizualizatorWidget::closeEvent(QCloseEvent *event)
{
    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__;
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
    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__;
    m_isCapturingImage = true;
    m_imageCapture->capture();
}

void VizualizatorWidget::slotReadyForCapture(bool ready)
{

    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__<< ready;
    ui->btnTakePicture->setEnabled(ready);
}

void VizualizatorWidget::slotImageExposed(int id)
{

    if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__<< id;
}

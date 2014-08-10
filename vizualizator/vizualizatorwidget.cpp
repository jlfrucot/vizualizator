#include "vizualizatorwidget.h"
#include "ui_vizualizatorwidget.h"

VizualizatorWidget::VizualizatorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VizualizatorWidget),
    m_camera(0),
    m_imageCapture(0),
    m_mediaRecorder(0),
    m_isCapturingImage(false),
    m_applicationExiting(false)
{
    m_localDebug = true;
    ui->setupUi(this);

    //Camera devices:
    QByteArray cameraDevice;
    m_videoDevicesGroup = new QActionGroup(this);
    m_videoDevicesGroup->setExclusive(true);
    foreach(const QByteArray &deviceName, QCamera::availableDevices())
    {
        QString description = m_camera->deviceDescription(deviceName);
        QAction *videoDeviceAction = new QAction(description, m_videoDevicesGroup);
        videoDeviceAction->setCheckable(true);
        videoDeviceAction->setData(QVariant(deviceName));
        if (cameraDevice.isEmpty())
        {
            cameraDevice = deviceName;
            videoDeviceAction->setChecked(true);
        }

    }
connect(m_videoDevicesGroup, SIGNAL(triggered(QAction*)), SLOT(updateCameraDevice(QAction*)));

setCamera(cameraDevice);
}

VizualizatorWidget::~VizualizatorWidget()
{
    delete ui;
}

void VizualizatorWidget::setCamera(const QByteArray &cameraDevice)
{
    delete m_imageCapture;
    delete m_mediaRecorder;
    delete m_camera;

    if (cameraDevice.isEmpty())
        m_camera = new QCamera;
    else
        m_camera = new QCamera(cameraDevice);

    connect(m_camera, SIGNAL(stateChanged(QCamera::State)), this, SLOT(updateCameraState(QCamera::State)));
    connect(m_camera, SIGNAL(error(QCamera::Error)), this, SLOT(displayCameraError()));

    m_mediaRecorder = new QMediaRecorder(m_camera);
    connect(m_mediaRecorder, SIGNAL(stateChanged(QMediaRecorder::State)), this, SLOT(updateRecorderState(QMediaRecorder::State)));

    m_imageCapture = new QCameraImageCapture(m_camera);

    connect(m_mediaRecorder, SIGNAL(durationChanged(qint64)), this, SLOT(updateRecordTime()));
    connect(m_mediaRecorder, SIGNAL(error(QMediaRecorder::Error)), this, SLOT(displayRecorderError()));

    m_mediaRecorder->setMetaData(QMediaMetaData::Title, QVariant(QLatin1String("Test Title")));


    m_camera->setViewfinder(ui->viewfinder);

    updateCameraState(m_camera->state());
    updateLockStatus(m_camera->lockStatus(), QCamera::UserRequest);
    updateRecorderState(m_mediaRecorder->state());

    connect(m_imageCapture, SIGNAL(readyForCaptureChanged(bool)), this, SLOT(readyForCapture(bool)));
    connect(m_imageCapture, SIGNAL(imageCaptured(int,QImage)), this, SLOT(processCapturedImage(int,QImage)));
    connect(m_imageCapture, SIGNAL(imageSaved(int,QString)), this, SLOT(imageSaved(int,QString)));
    connect(m_imageCapture, SIGNAL(error(int,QCameraImageCapture::Error,QString)), this,
            SLOT(displayCaptureError(int,QCameraImageCapture::Error,QString)));

    connect(m_camera, SIGNAL(lockStatusChanged(QCamera::LockStatus, QCamera::LockChangeReason)),
            this, SLOT(updateLockStatus(QCamera::LockStatus, QCamera::LockChangeReason)));

//    ui->captureWidget->setTabEnabled(0, (camera->isCaptureModeSupported(QCamera::CaptureStillImage)));
//    ui->captureWidget->setTabEnabled(1, (camera->isCaptureModeSupported(QCamera::CaptureVideo)));

    updateCaptureMode();
    m_camera->start();
}
void VizualizatorWidget::updateCameraDevice(QAction *action)
{
     if (m_localDebug) qDebug()<<" ++++++++ " << __FUNCTION__<<action->text();
    setCamera(action->data().toByteArray());
}
void VizualizatorWidget::updateRecordTime()
{
    QString str = QString("Recorded %1 sec").arg(m_mediaRecorder->duration()/1000);
//    ui->statusbar->showMessage(str);
}

void VizualizatorWidget::processCapturedImage(int requestId, const QImage& img)
{
    Q_UNUSED(requestId);
    QImage scaledImage = img.scaled(ui->viewfinder->size(),
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation);

//    ui->lastImagePreviewLabel->setPixmap(QPixmap::fromImage(scaledImage));

//    // Display captured image for 4 seconds.
//    displayCapturedImage();
//    QTimer::singleShot(4000, this, SLOT(displayViewfinder()));
}

void VizualizatorWidget::configureCaptureSettings()
{
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
//    ImageSettings settingsDialog(imageCapture);

//    settingsDialog.setImageSettings(imageSettings);

//    if (settingsDialog.exec()) {
//        m_imageSettings = settingsDialog.imageSettings();
//        m_imageCapture->setEncodingSettings(imageSettings);
//    }
}

void VizualizatorWidget::record()
{
    m_mediaRecorder->record();
    updateRecordTime();
}

void VizualizatorWidget::pause()
{
    m_mediaRecorder->pause();
}

void VizualizatorWidget::stop()
{
    m_mediaRecorder->stop();
}

void VizualizatorWidget::setMuted(bool muted)
{
    m_mediaRecorder->setMuted(muted);
}

void VizualizatorWidget::toggleLock()
{
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
    m_isCapturingImage = true;
    m_imageCapture->capture();
}

void VizualizatorWidget::displayCaptureError(int id, const QCameraImageCapture::Error error, const QString &errorString)
{
    Q_UNUSED(id);
    Q_UNUSED(error);
//    QMessageBox::warning(this, tr("Image Capture Error"), errorString);
    m_isCapturingImage = false;
}

void VizualizatorWidget::startCamera()
{
    m_camera->start();
}

void VizualizatorWidget::stopCamera()
{
    m_camera->stop();
}

void VizualizatorWidget::updateCaptureMode()
{
//    int tabIndex = ui->captureWidget->currentIndex();
//    QCamera::CaptureModes captureMode = tabIndex == 0 ? QCamera::CaptureStillImage : QCamera::CaptureVideo;

//    if (camera->isCaptureModeSupported(captureMode))
//        camera->setCaptureMode(captureMode);
}

void VizualizatorWidget::updateCameraState(QCamera::State state)
{
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

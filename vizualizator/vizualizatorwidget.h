#ifndef VIZUALIZATORWIDGET_H
#define VIZUALIZATORWIDGET_H

#include <QWidget>
#include <QMediaService>
#include <QMediaRecorder>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QMediaMetaData>
#include <QActionGroup>
#include <QCamera>

namespace Ui {
class VizualizatorWidget;
}

class VizualizatorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VizualizatorWidget(QWidget *parent = 0);
    ~VizualizatorWidget();
    inline QActionGroup * vizualizatorWidgetGetVideoDevicesGroup(){return m_videoDevicesGroup;}
private slots:
    void updateCameraDevice(QAction *action);
private:
    bool m_localDebug;
    Ui::VizualizatorWidget *ui;
    QCamera *m_camera;
    QCameraImageCapture *m_imageCapture;
    QMediaRecorder* m_mediaRecorder;

    QImageEncoderSettings m_imageSettings;
    QAudioEncoderSettings m_audioSettings;
    QVideoEncoderSettings m_videoSettings;
    QString m_videoContainerFormat;
    bool m_isCapturingImage;
    bool m_applicationExiting;
    QActionGroup *m_videoDevicesGroup;
    void setCamera(const QByteArray &cameraDevice);
    void configureCaptureSettings();
    void configureVideoSettings();
    void configureImageSettings();
    void record();
    void pause();
    void stop();
    void setMuted(bool muted);
    void toggleLock();
    void updateLockStatus(QCamera::LockStatus status, QCamera::LockChangeReason reason);
    void takeImage();
    void displayCaptureError(int id, const QCameraImageCapture::Error error, const QString &errorString);
    void startCamera();
    void stopCamera();
    void updateCaptureMode();
    void updateCameraState(QCamera::State state);
    void updateRecorderState(QMediaRecorder::State state);
    void updateRecordTime();
    void processCapturedImage(int requestId, const QImage &img);
};

#endif // VIZUALIZATORWIDGET_H

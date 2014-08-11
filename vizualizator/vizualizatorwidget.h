#ifndef VIZUALIZATORWIDGET_H
#define VIZUALIZATORWIDGET_H

#include <QWidget>
#include <QTransform>
#include <QMessageBox>
#include <QCloseEvent>
#include <QMediaService>
#include <QMediaRecorder>
#include <QCameraViewfinder>
#include <QGraphicsScene>
#include <QGraphicsVideoItem>
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
    void updateCaptureMode();
    void updateCameraState(QCamera::State state);
    void updateRecorderState(QMediaRecorder::State state);
    void updateRecordTime();
    void updateLockStatus(QCamera::LockStatus status, QCamera::LockChangeReason reason);
    void processCapturedImage(int requestId, const QImage &img);
    void displayCaptureError(int id, const QCameraImageCapture::Error error, const QString &errorString);
    void displayRecorderError();
    void displayCameraError();
//    void readyForCapture(bool ready);
    void imageSaved(int id, const QString &fileName);
    void closeEvent(QCloseEvent *event);
    void on_btnTakePicture_clicked();
    void slotReadyForCapture(bool ready);
    void slotImageExposed(int id);
private:
    bool m_localDebug;
    Ui::VizualizatorWidget *ui;
    QGraphicsScene *m_scene;
    QCamera *m_camera;
    QCameraImageCapture *m_imageCapture;
    QGraphicsVideoItem *m_viewfinder;
    QMediaRecorder* m_mediaRecorder;

    QTransform m_transform;
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
    void takeImage();
    void startCamera();
    void stopCamera();
};

#endif // VIZUALIZATORWIDGET_H

#ifndef VIZUALIZATORWIDGET_H
#define VIZUALIZATORWIDGET_H

#include <QApplication>
#include <QDesktopWidget>
#include <QSize>
#include <QWidget>
#include <QLabel>
#include <QToolBox>
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
#include <QTimer>
#include <QListWidget>
#include <QHideEvent>

#include "lib/vizualizatorimage.h"
namespace Ui {
class VizualizatorWidget;
}
///
/// \brief Ce widget affiche les images générées par les webcam connectées au système
///
/// Les images issues de la caméra sélectionnée, peuvent être capturées et enregistrées soit
/// directement dans le dossier des images de Sankoré (logiciel de TBI), soit ailleurs.
///
class VizualizatorWidget : public QWidget
{
    Q_OBJECT

    Q_ENUMS(ThumbnailRole)
//    Q_FLAGS(Flip Flips)
public:

    enum ThumbnailRole{ImagePointer = Qt::UserRole+1, FilePath = Qt::UserRole+2, Rotation = Qt::UserRole+3, FlipXaxis = Qt::UserRole+4, FlipYaxis = Qt::UserRole+5};
//    enum Flip{FlipHorizontal = 1, FlipVertical = 2};
//    Q_DECLARE_FLAGS(Flips, Flip)
    explicit VizualizatorWidget(QWidget *parent = 0);
    ~VizualizatorWidget();
    ///
    /// \brief Fournit un pointeur vers un QActionGroup présentant la liste des caméras connectées
    ///
    /// \return La liste des caméras connectées sous forme d'un QActionGroup
    /// \code
    ///     /* On place la liste des QActions dans le menu Caméras de la mainwindow */
    ///     ui->menuCamera->addActions(ui->widgetVizu->vizualizatorWidgetGetVideoDevicesGroup()->actions());
    /// \endcode
    ///
    inline QActionGroup * vizualizatorWidgetGetVideoDevicesGroup(){return m_videoDevicesGroup;}

    ///
    /// \brief Fournit un pointeur vers la QToolBox du vizualizatorWidget
    ///
    /// Permet de placer la ToolBox à un autre endroit tel qu'un dock widget dans la mainwindow
    /// \code
    ///ui->dockWidget->setWidget(ui->widgetVizu->VizualizatorWidgetGetToolBox());
    /// \endcode
    /// \return Pointeur vers QToolBox
    ///
    QWidget *VizualizatorWidgetGetToolBox();
    QByteArray vizualizatorGetCameras();
    void restoreUiFromItem(QListWidgetItem *item);
    void showFullScreen(QImage scaledImage);
    int rotation() const;
    void setRotation(int rotation);

    bool isFlipXaxis() const;
    void setFlipXaxis(bool flipXaxis);

    bool isFlipYaxis() const;
    void setFlipYaxis(bool flipYaxis);

    QSize getIconSize() const;
    void setIconSize(const QSize &iconSize);

private slots:

    void showResizedImage();
    ///
    /// \brief Change de webcam
    /// \param action La caméra sélectionnée dans le menu
    ///
    void updateCameraDevice(QAction *action);
    void updateCaptureMode();
    void updateCameraState(QCamera::State state);
    void updateLockStatus(QCamera::LockStatus status, QCamera::LockChangeReason reason);

    void on_btnTakePicture_clicked();
    void slotReadyForCapture(bool ready);
    void displayCaptureError(int id, const QCameraImageCapture::Error error, const QString &errorString);
    void processCapturedImage(int requestId, const QImage &img);
    void displayCameraError();
    void slotImageExposed(int id);
    void imageSaved(int id, const QString &fileName);

    void updateRecorderState(QMediaRecorder::State state);
    void updateRecordTime();
    void displayRecorderError();

    //////////////////////////////////////////////////////////////////////////
    /// Les boutons de changement d'orientation et de retournement de l'image
    /// de la caméra
    //////////////////////////////////////////////////////////////////////////
    void on_dialOrientation_valueChanged(int value);
    void on_rbRotate0deg_clicked(bool checked);
    void on_rbRotate90deg_clicked(bool checked);
    void on_rbRotate180deg_clicked(bool checked);
    void on_rbRotate270deg_clicked(bool checked);
    void on_btnYaxisMirror_clicked(bool checked);
    void on_btnXaxisMirror_clicked(bool checked);

    /// Met à jour les transformations appliquées au viewfinder
    void updateViewfinderTransformations();


    void resizeEvent(QResizeEvent *);
    void showEvent(QShowEvent *);
    void closeEvent(QCloseEvent *event);
    void on_cbNativeImage_clicked(bool checked);

    void on_btnFullScreenImage_clicked();

    void hideEvent(QHideEvent *);
    void on_tabWidget_currentChanged(int index);

    void on_tbToolPanel_currentChanged(int index);

    void on_lwGallery_itemClicked(QListWidgetItem *item);

    void slotUpdateThumbnailItem();
    void on_cBoxSelectCam_currentIndexChanged(int index);

    void on_vSBrightness_valueChanged(int value);
    
private:
    bool m_localDebug;
    Ui::VizualizatorWidget *ui;
    int m_rotation;
    bool m_flipXaxis;
    bool m_flipYaxis;
    QSize m_iconSize;
    QGraphicsScene *m_scene;
    QGraphicsScene *m_sceneImage;
    QCamera *m_camera;
    QCameraImageCapture *m_imageCapture;
    QGraphicsPixmapItem *m_imageItem;
    VizualizatorImage *m_currentImage;
    QList<QLabel *>m_fullscreenLabels;
    QGraphicsVideoItem *m_viewfinder;
    QMediaRecorder* m_mediaRecorder;

    QTransform m_transformRotation;
    QTransform m_transformYaxisMirror;
    QTransform m_transformXaxisMirror;
    QList<QTransform> m_transformations;
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
//    void takeImage();
    void startCamera();
    void stopCamera();

};

/// @brief conversion de pointer void* en Qvariant et vice versa
/// @code
/// MyClass *p;
/// QVariant v = VariantPtr<MyClass>::asQVariant(p);
/// /* Retrouver le pointer */
/// MyClass *p1 = VariantPtr<MyClass>::asPtr(v);
/// @endcode
template <class T> class VariantPtr
{
public:
    /* Retrouver le pointeur */
    static T* asPtr(QVariant v)
    {
        return  (T *) v.value<void *>();
    }
    /* Transformer le pointeur en QVariant */
    static QVariant asQVariant(T* ptr)
    {
        return qVariantFromValue((void *) ptr);
    }
};


#endif // VIZUALIZATORWIDGET_H

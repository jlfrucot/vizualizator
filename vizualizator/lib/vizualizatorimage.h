#ifndef VIZUALIZATORIMAGE_H
#define VIZUALIZATORIMAGE_H

#include <QImage>
#include <QDebug>
#include <QMatrix>
#ifdef WITH_OPENCV2
  #include <opencv2/highgui/highgui.hpp>
  #include <opencv2/imgproc/imgproc.hpp>
  #include <opencv2/core/core.hpp>
#endif

///
/// \brief LA classe VizualizatorImage décrit une image avec toutes ses propriétés
///
class VizualizatorImage : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int AngleRotation    MEMBER m_angleRotation  READ getAngleRotation   WRITE setAngleRotation)
    Q_PROPERTY(bool FlipXaxis       MEMBER m_flipXaxis      READ isFlipXaxis        WRITE setFlipXaxis)
    Q_PROPERTY(bool FlipYaxis       MEMBER m_flipYaxis      READ isFlipYaxis        WRITE setFlipYaxis)
    Q_PROPERTY(QSize getIconeSize   MEMBER m_iconSize       READ getIconeSize       WRITE setIconeSize)
    Q_PROPERTY(QString PathImage    MEMBER m_pathImage      READ getPathImage       WRITE setPathImage)
    Q_PROPERTY(qreal BrightnessValue MEMBER m_brightnessValue READ getBrightnessValue WRITE setBrightnessValue)
    Q_PROPERTY(qreal ContrastValue  MEMBER m_contrastValue  READ getContrastValue WRITE setContrastValue)
public:
    explicit VizualizatorImage(const QImage &image);
    ~VizualizatorImage();

    inline QImage const getOriginalImage(){return m_image;}
    ///
    /// \brief Return la QImage tournée de n degrés
    /// \return QImage tournée
    ///
    QImage getRotatedImage();


    QImage getThumbnail();

    int     getAngleRotation() const;
    void    setAngleRotation(int angleRotation);

    bool    isFlipXaxis() const;
    void    setFlipXaxis(bool flipXaxis);

    bool    isFlipYaxis() const;
    void    setFlipYaxis(bool flipYaxis);

    QSize   getIconSize() const;
    void    setIconSize(const QSize &iconSize);

    QString getPathImage() const;
    void    setPathImage(const QString &pathImage);

    qreal getBrightnessValue() const;
    void setBrightnessValue(const qreal &getBrightnessValue);

    qreal getContrastValue() const;
    void setContrastValue(const qreal &contrastValue);

private:
    bool         m_localDebug;
    const QImage m_image;
    int          m_angleRotation;
    bool         m_flipXaxis;
    bool         m_flipYaxis;
    QSize        m_iconSize;
    QString      m_pathImage;
    qreal        m_brightnessValue;
    qreal        m_contrastValue;
    QImage  cvMatToQimage(const cv::Mat &mat, QImage::Format format);
    cv::Mat qImageToCvMat(const QImage &inImage, bool inCloneImageData = true);
    void updateContrastAndBrightness(const cv::Mat srcMat, cv::Mat desMat, qreal contrastValue, qreal brightnessValue);
signals:

public slots:

};
#endif // VIZUALIZATORIMAGE_H

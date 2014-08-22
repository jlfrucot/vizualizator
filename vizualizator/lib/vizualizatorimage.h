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
class VizualizatorImage
{
public:
    explicit VizualizatorImage(const QImage &image);
    ~VizualizatorImage();

    inline QImage const getOriginalImage(){return m_image;}
    ///
    /// \brief Return la QImage tournée de n degrés
    /// \param angle
    /// \return QImage tournée
    ///
    QImage getRotatedImage();

    QImage getThumbnail();
    int getAngleRotation() const;
    void setAngleRotation(int getAngleRotation);

    bool isFlipXaxis() const;
    void setFlipXaxis(bool flipXaxis);

    bool isFlipYaxis() const;
    void setFlipYaxis(bool isFlipYaxis);

    QSize getIconSize() const;
    void  setIconSize(const QSize &iconSize);

private:
    bool         m_localDebug;
    const QImage m_image;
    int          m_angleRotation;
    bool         m_flipXaxis;
    bool         m_flipYaxis;
    QSize        m_iconSize;
    QImage cvMatToQimage(const cv::Mat &mat, QImage::Format format);
    cv::Mat qImageToCvMat(const QImage &inImage, bool inCloneImageData = true);
signals:

public slots:

};
#endif // VIZUALIZATORIMAGE_H

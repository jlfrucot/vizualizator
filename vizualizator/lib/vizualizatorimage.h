#ifndef VIZUALIZATORIMAGE_H
#define VIZUALIZATORIMAGE_H

#include <QImage>
#include <QDebug>
#ifdef WITH_OPENCV2
  #include <opencv2/highgui/highgui.hpp>
  #include <opencv2/imgproc/imgproc.hpp>
  #include <opencv2/core/core.hpp>
#endif
class VizualizatorImage : public QImage
{

public:
    explicit VizualizatorImage(const QImage &image);
    ~VizualizatorImage();
    ///
    /// \brief Return la QImage tournée de n degrés
    /// \param angle
    /// \return QImage tournée
    ///
    QImage rotate(int angle);

    inline QImage getOriginalImage(){return m_image;}
private:
    const QImage m_image;
    int m_angle;
    QImage cvMatToQimage(const cv::Mat &mat, QImage::Format format);
    cv::Mat qImageToCvMat(const QImage &inImage, bool inCloneImageData = true);
    bool m_localDebug;
signals:

public slots:

};

#endif // VIZUALIZATORIMAGE_H

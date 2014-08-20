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
    QImage getRotatedImage(int angle, bool xAxisMirror = false, bool yAxisMirror = false);

    QImage getThumbnail(QSize size = QSize(128,128), int rotation = 0, bool xAxisMirror = false, bool yAxisMirror = false );
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

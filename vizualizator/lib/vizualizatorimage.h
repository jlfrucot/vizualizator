#ifndef VIZUALIZATORIMAGE_H
#define VIZUALIZATORIMAGE_H

#include <QImage>
#include <QDebug>
#ifdef WITH_OPENCV2
  #include <opencv2/highgui/highgui.hpp>
  #include <opencv2/imgproc/imgproc.hpp>
#endif
class VizualizatorImage : public QImage
{
//    Q_OBJECT
public:
    explicit VizualizatorImage(const QImage &image, QObject *parent = 0);

    ///
    /// \brief Return la QImage tournée de n degrés
    /// \param angle
    /// \return QImage tournée
    ///
    QImage rotate(int angle);
private:
    const QImage m_image;
    int m_angle;
    QImage cvMatToQimage(cv::Mat &mat, QImage::Format format);
    cv::Mat qImageToCvMat(const QImage &inImage, bool inCloneImageData = true);
signals:

public slots:

};

#endif // VIZUALIZATORIMAGE_H

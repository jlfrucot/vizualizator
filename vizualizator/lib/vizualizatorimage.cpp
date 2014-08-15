#include "vizualizatorimage.h"

VizualizatorImage::VizualizatorImage(const QImage &image, QObject *parent) :
    QImage(image),
    m_image(image),
    m_angle(0)
{

}

QImage VizualizatorImage::rotate(int angle)
{
    cv::Mat mat(qImageToCvMat(m_image, false));
    cv::Mat matRotated = cv::Mat::zeros(mat.rows,mat.cols, mat.type());
        cv::Point2f pt(mat.cols/2., mat.rows/2.);
        cv::Mat r = cv::getRotationMatrix2D(pt, angle, 1.0);
        cv::warpAffine(mat, matRotated, r, matRotated.size());
    return cvMatToQimage(matRotated, m_image.format());
}

/* Convertit une cv::Mat en QImage avec copie */
QImage VizualizatorImage::cvMatToQimage(cv::Mat &mat, QImage::Format format)
{
  return QImage(mat.data, mat.cols, mat.rows, mat.step, format).copy();
}

cv::Mat VizualizatorImage::qImageToCvMat( const QImage &inImage, bool inCloneImageData)
   {
      switch ( inImage.format() )
      {
         // 8-bit, 4 channel
         case QImage::Format_RGB32:
         {
          qDebug()<<"QImage::Format_RGB32";
            cv::Mat  mat( inImage.height(), inImage.width(), CV_8UC4, const_cast<uchar*>(inImage.bits()), inImage.bytesPerLine() );
            return (inCloneImageData ? mat.clone() : mat);
         }

         // 8-bit, 3 channel
         case QImage::Format_RGB888:
         {
          qDebug()<<"QImage::Format_RGB888";
          cv::Mat  mat( inImage.height(), inImage.width(), CV_8UC3, const_cast<uchar*>(inImage.bits()), inImage.bytesPerLine() );
          return (inCloneImageData ? mat.clone() : mat);
         }

         // 8-bit, 1 channel
         case QImage::Format_Indexed8:
         {
          qDebug()<<"QImage::Format_Indexed8";
            cv::Mat  mat( inImage.height(), inImage.width(), CV_8UC1, const_cast<uchar*>(inImage.bits()), inImage.bytesPerLine() );
            return (inCloneImageData ? mat.clone() : mat);
         }

         default:
            qWarning() << "QImage format not handled in switch:" << inImage.format();
            break;
      }

      return cv::Mat();
   }

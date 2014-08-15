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
    cv::Mat matRotated;
    /* On crée une matrice dont les dimension sont celle d'un carré de coté la diagonale de l'image
     * Comme cela l'image tournée tiendra dedans sans cropping
     */
    int diagonal = (int)sqrt(mat.cols*mat.cols+mat.rows*mat.rows);
    int newWidth = diagonal;
    int newHeight =diagonal;

    int offsetX = (newWidth - mat.cols) / 2;
    int offsetY = (newHeight - mat.rows) / 2;
    cv::Mat targetMat(newWidth, newHeight, mat.type());
    cv::Point2f src_center(targetMat.cols/2.0F, targetMat.rows/2.0F);

    double radians = angle * M_PI / 180.0;
    double sin = abs(std::sin(radians));
    double cos = abs(std::cos(radians));

    /* On copie l'image dans la grande image carrée, au mileu */
    cv::Mat dst_roi = targetMat.rowRange(offsetY, offsetY + mat.rows).colRange(offsetX, offsetX + mat.cols);
    mat.copyTo(dst_roi);
    /* Calcul de la matrice de rotation */
    cv::Mat r = cv::getRotationMatrix2D(src_center, angle, 1.0);
    /* Rotation l'image tournée est dans matRotated */
    cv::warpAffine(targetMat, matRotated, r, targetMat.size());

    /* On enlève ce qui est en trop */

    //Calculate bounding rect and for exact image
     //Reference:- http://stackoverflow.com/questions/19830477/find-the-bounding-rectangle-of-rotated-rectangle/19830964?noredirect=1#19830964
        cv::Rect bound_Rect(mat.cols,mat.rows,0,0);

        int x1 = offsetX;
        int x2 = offsetX+mat.cols;
        int x3 = offsetX;
        int x4 = offsetX+mat.cols;

        int y1 = offsetY;
        int y2 = offsetY;
        int y3 = offsetY+mat.rows;
        int y4 = offsetY+mat.rows;

        cv::Mat co_Ordinate = (cv::Mat_<double>(3,4) << x1, x2, x3, x4,
                                                y1, y2, y3, y4,
                                                 1,  1,  1,  1 );
        cv::Mat RotCo_Ordinate = r * co_Ordinate;

        for(int i=0;i<4;i++){
           if(RotCo_Ordinate.at<double>(0,i)<bound_Rect.x)
             bound_Rect.x=(int)RotCo_Ordinate.at<double>(0,i); //access smallest
           if(RotCo_Ordinate.at<double>(1,i)<bound_Rect.y)
            bound_Rect.y=RotCo_Ordinate.at<double>(1,i); //access smallest y
         }

         for(int i=0;i<4;i++){
           if(RotCo_Ordinate.at<double>(0,i)>bound_Rect.width)
             bound_Rect.width=(int)RotCo_Ordinate.at<double>(0,i); //access largest x
           if(RotCo_Ordinate.at<double>(1,i)>bound_Rect.height)
            bound_Rect.height=RotCo_Ordinate.at<double>(1,i); //access largest y
         }

        bound_Rect.width=bound_Rect.width-bound_Rect.x;
        bound_Rect.height=bound_Rect.height-bound_Rect.y;

        cv::Mat cropedResult;
        cv::Mat ROI = matRotated(bound_Rect);
        ROI.copyTo(cropedResult);

    return cvMatToQimage(cropedResult, m_image.format());
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

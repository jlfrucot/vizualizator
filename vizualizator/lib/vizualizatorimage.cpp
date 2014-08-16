#include "vizualizatorimage.h"

VizualizatorImage::VizualizatorImage(const QImage &image) :
    QImage(image),
    m_image(image),
    m_angle(0),
    m_localDebug(false)
{

}

QImage VizualizatorImage::rotate(int angle)
{
    cv::Mat matOriginal(qImageToCvMat(m_image, false));
    cv::Mat matRotated;
    /* On crée une matrice dont les dimensions sont celles d'un carré de côté la diagonale de l'image
     * Comme cela l'image tournée tiendra dedans sans cropping
     */
    int diagonal = (int)sqrt(matOriginal.cols*matOriginal.cols+matOriginal.rows*matOriginal.rows);

    int offsetX = (diagonal - matOriginal.cols) / 2;
    int offsetY = (diagonal - matOriginal.rows) / 2;
    cv::Mat targetMat(diagonal, diagonal, matOriginal.type());
    cv::Point2f imageCenter(targetMat.cols/2.0F, targetMat.rows/2.0F);

    /* On copie l'image dans la grande image carrée, au mileu
     * Il faut le faire en deux étapes car la syntaxe n'est pas la même suivant la version de opencv2
     * ROI Region Of Interest selon opencv
     */
    cv::Mat targetMatROI = targetMat.rowRange(offsetY, offsetY + matOriginal.rows).colRange(offsetX, offsetX + matOriginal.cols);
    matOriginal.copyTo(targetMatROI);
    /* Calcul de la matrice de rotation */
    cv::Mat rotateMat = cv::getRotationMatrix2D(imageCenter, angle, 1.0);
    /* Rotation l'image tournée est dans matRotated */
    cv::warpAffine(targetMat, matRotated, rotateMat, targetMat.size());

    /* On enlève ce qui est en trop :
     * On va calculer le boundingrect minimum de l'image tournée
     */

    //Calculate bounding rect and for exact image
    //Reference:- http://stackoverflow.com/questions/19830477/find-the-bounding-rectangle-of-rotated-rectangle/19830964?noredirect=1#19830964
    cv::Rect bound_Rect(matOriginal.cols,matOriginal.rows,0,0);

    int x1 = offsetX;
    int x2 = offsetX+matOriginal.cols;
    int x3 = offsetX;
    int x4 = offsetX+matOriginal.cols;

    int y1 = offsetY;
    int y2 = offsetY;
    int y3 = offsetY+matOriginal.rows;
    int y4 = offsetY+matOriginal.rows;

    cv::Mat vertexCoordinate = (cv::Mat_<double>(3,4) << x1, x2, x3, x4,
                                                         y1, y2, y3, y4,
                                                          1,  1,  1,  1 );
    /* On applique la matrice de rotation aux quatres points représentés dans la matrice ci-dessus */
    cv::Mat vertexCoordinateRotated = rotateMat * vertexCoordinate;
    /* On cherche les x et y max et min pour avoir les coordonnées du boundingrect */

    /* TODO Optimiser en fonction de l'angle de rotation
         * On peut connaitre directement les valeurs max et min de x et y
         * Plutôt que d'avoir 8 tours de boucle à faire.
         */
    for(int i=0;i<4;i++){
        if(vertexCoordinateRotated.at<double>(0,i)<bound_Rect.x)
            bound_Rect.x=(int)vertexCoordinateRotated.at<double>(0,i); //access smallest x
        if(vertexCoordinateRotated.at<double>(1,i)<bound_Rect.y)
            bound_Rect.y=vertexCoordinateRotated.at<double>(1,i); //access smallest y
    }

    for(int i=0;i<4;i++){
        if(vertexCoordinateRotated.at<double>(0,i)>bound_Rect.width)
            bound_Rect.width=(int)vertexCoordinateRotated.at<double>(0,i); //access largest x
        if(vertexCoordinateRotated.at<double>(1,i)>bound_Rect.height)
            bound_Rect.height=vertexCoordinateRotated.at<double>(1,i); //access largest y
    }

    bound_Rect.width=bound_Rect.width-bound_Rect.x;
    bound_Rect.height=bound_Rect.height-bound_Rect.y;

    cv::Mat croppedResult;
    cv::Mat ROI = matRotated(bound_Rect);
    ROI.copyTo(croppedResult);

    return cvMatToQimage(croppedResult, m_image.format());
}

/* Convertit une cv::Mat en QImage avec copie */
QImage VizualizatorImage::cvMatToQimage(const cv::Mat &mat, QImage::Format format)
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

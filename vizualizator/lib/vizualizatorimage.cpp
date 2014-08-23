#include "vizualizatorimage.h"

VizualizatorImage::VizualizatorImage(const QImage &image) :
    m_localDebug(false),
    m_image(image),
    m_angleRotation(0),
    m_flipXaxis(false),
    m_flipYaxis(false),
    m_iconSize(QSize(128,128)),
    m_pathImage(QString()),
    m_brightnessValue(0.0),
    m_contrastValue(1.0)
{
}

VizualizatorImage::~VizualizatorImage()
{

}

QImage VizualizatorImage::getRotatedImage()
{
    cv::Mat matOriginal(qImageToCvMat(m_image, true)); // Avec copie pour ne pas modifier l'image de base
    cv::Mat matRotated;
    if(m_flipXaxis)
    {
        cv::flip(matOriginal, matOriginal,0); // xAxis
    }
    if(m_flipYaxis)
    {
        cv::flip(matOriginal, matOriginal,1); // yAxis
    }
    if(m_brightnessValue || m_contrastValue != 1)
    {
        updateContrastAndBrightness(matOriginal, matOriginal, m_contrastValue, m_brightnessValue);
    }
    if(m_angleRotation)
    {
    /* On crée une matrice dont les dimensions sont celles d'un carré de côté la diagonale de l'image
     * Comme cela l'image tournée tiendra dedans sans cropping
     */
    int diagonal = (int)sqrt(matOriginal.cols*matOriginal.cols+matOriginal.rows*matOriginal.rows);

    int offsetX = (diagonal - matOriginal.cols) / 2;
    int offsetY = (diagonal - matOriginal.rows) / 2;
    cv::Mat targetMat;
    targetMat = cv::Mat::zeros(diagonal, diagonal, matOriginal.type());
    cv::Point2f imageCenter(targetMat.cols/2.0F, targetMat.rows/2.0F);

    /* On copie l'image dans la grande image carrée, au mileu
     * Il faut le faire en deux étapes car la syntaxe n'est pas la même suivant la version de opencv2
     * ROI Region Of Interest selon opencv
     */
    cv::Mat targetMatROI = targetMat.rowRange(offsetY, offsetY + matOriginal.rows).colRange(offsetX, offsetX + matOriginal.cols);
    matOriginal.copyTo(targetMatROI);
    /* Calcul de la matrice de rotation */
    cv::Mat rotateMat = cv::getRotationMatrix2D(imageCenter, 360-m_angleRotation, 1.0);
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
    matOriginal = croppedResult;
    }
    return cvMatToQimage(matOriginal, m_image.format());
}

QImage VizualizatorImage::getThumbnail( )
{
    QImage thumbnail = m_image.scaled(m_iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if(m_flipXaxis)
    {
        thumbnail = thumbnail.mirrored(false, true);
    }
    if(m_flipYaxis)
    {
        thumbnail = thumbnail.mirrored(true, false);
    }
    if(m_angleRotation)
    {
        QMatrix matrixRotate;
        matrixRotate.reset();
        matrixRotate.rotate(m_angleRotation);
        thumbnail = thumbnail.transformed(matrixRotate);
    }
    return thumbnail;
}
int VizualizatorImage::getAngleRotation() const
{
    return m_angleRotation;
}

void VizualizatorImage::setAngleRotation(int angleRotation)
{
    m_angleRotation = angleRotation;
}
bool VizualizatorImage::isFlipXaxis() const
{
    return m_flipXaxis;
}

void VizualizatorImage::setFlipXaxis(bool flipXaxis)
{
    m_flipXaxis = flipXaxis;
}
bool VizualizatorImage::isFlipYaxis() const
{
    return m_flipYaxis;
}

void VizualizatorImage::setFlipYaxis(bool flipYaxis)
{
    m_flipYaxis = flipYaxis;
}

QSize VizualizatorImage::getIconSize() const
{
    return m_iconSize;
}

void VizualizatorImage::setIconSize(const QSize &iconSize)
{
    m_iconSize = iconSize;
}
QString VizualizatorImage::getPathImage() const
{
    return m_pathImage;
}

void VizualizatorImage::setPathImage(const QString &pathImage)
{
    m_pathImage = pathImage;
}
qreal VizualizatorImage::getBrightnessValue() const
{
    return m_brightnessValue;
}

void VizualizatorImage::setBrightnessValue(const qreal &brightnessValue)
{
    m_brightnessValue = brightnessValue;
}
qreal VizualizatorImage::getContrastValue() const
{
    return m_contrastValue;
}

void VizualizatorImage::setContrastValue(const qreal &contrastValue)
{
    m_contrastValue = contrastValue;
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

void VizualizatorImage::updateContrastAndBrightness(const cv::Mat srcMat, cv::Mat desMat,qreal contrastValue, qreal brightnessValue)
{
    qDebug()<<"updateBrightness()";
    srcMat.convertTo(desMat, -1, contrastValue, brightnessValue);
}

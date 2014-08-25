#ifndef VIZUALIZATORIMAGE_H
#define VIZUALIZATORIMAGE_H

#include <cmath>
#include <QImage>
#include <QSettings>
#include <QMetaProperty>
#include <QDebug>
#include <QMatrix>
#ifdef WITH_OPENCV2
  #include <opencv2/core/core.hpp>
  #include <opencv2/highgui/highgui.hpp>
  #include <opencv2/imgproc/imgproc.hpp>
#endif

///
/// \brief La classe VizualizatorImage décrit une image avec toutes ses propriétés
///
class VizualizatorImage : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int AngleRotation    MEMBER m_angleRotation  READ getAngleRotation   WRITE setAngleRotation)
    Q_PROPERTY(bool FlipXaxis       MEMBER m_flipXaxis      READ isFlipXaxis        WRITE setFlipXaxis)
    Q_PROPERTY(bool FlipYaxis       MEMBER m_flipYaxis      READ isFlipYaxis        WRITE setFlipYaxis)
    Q_PROPERTY(QSize IconSize   MEMBER m_iconSize          READ getIconSize       WRITE setIconSize)
    Q_PROPERTY(QString PathImage    MEMBER m_pathImage      READ getPathImage       WRITE setPathImage)
    Q_PROPERTY(qreal BrightnessValue MEMBER m_brightnessValue READ getBrightnessValue WRITE setBrightnessValue)
    Q_PROPERTY(qreal ContrastValue  MEMBER m_contrastValue  READ getContrastValue WRITE setContrastValue)

public:
    explicit VizualizatorImage(const QImage &image);
    ~VizualizatorImage();

    inline QImage const getOriginalImage(){return m_image;}
    ///
    /// \brief Return la QImage modifiée
    /// \return QImage Modifiée
    ///
    QImage getModifiedImage();

    ///
    /// \brief retourne un thumbnail de l'image
    /// \return L'imagette de l'image modifiée ou pas
    ///
    QImage getThumbnail();

    ///////////////////////////////////////////////////////////
    ///       Les setters et accesseurs des propriétés
    ///////////////////////////////////////////////////////////

    ///
    /// \brief Retourne l'angle de rotation
    /// \return L'angle de rotation en degrés
    ///
    int     getAngleRotation() const;

    ///
    /// \brief Fixe l'angle de rotation
    /// \param angleRotation L'angle de rotation en degrés
    ///
    void    setAngleRotation(int angleRotation);

    ///
    /// \brief L'image a-t-elle subi une rotation de 180° suivant l'axe des X
    /// \return bool true or false
    ///
    bool    isFlipXaxis() const;

    ///
    /// \brief Fixe la rotation suivant l'axe des X (180°)
    /// \param flipXaxis true or false
    ///
    void    setFlipXaxis(bool flipXaxis);

    ///
    /// \brief L'image a-t-elle subi une rotation de 180° suivant l'axe des Y
    /// \return bool true or false
    ///
    bool    isFlipYaxis() const;

    ///
    /// \brief Fixe la rotation suivant l'axe des Y (180°)
    /// \param flipYaxis true or false
    ///
    void    setFlipYaxis(bool flipYaxis);

    ///
    /// \brief Retourne la taille du thumbnail (128x128 par défaut)
    /// \return La taille du Thumbnail
    ///
    QSize   getIconSize() const;

    ///
    /// \brief Fixe la taille du Thumbnail (128x128 par defaut)
    /// \param iconSize La taille du Thumbnail
    ///
    void    setIconSize(const QSize &iconSize);

    ///
    /// \brief Retourne le chemin de l'image enregistrée
    /// \return Le chemin de l'image enregistrée
    ///
    QString getPathImage() const;

    ///
    /// \brief Fixe le chemin où l'image a été enregistrée
    /// \param pathImage Le chemin de l'image enregistrée
    ///
    void    setPathImage(const QString &pathImage);

    ///
    /// \brief Retourne la valeur de la luminosité
    /// \return La valeur de la luminosité
    ///
    qreal getBrightnessValue() const;

    ///
    /// \brief Fixe la valeur de la luminosité
    /// \param brightnessValue La valeur de la luminosité
    ///
    void setBrightnessValue(const qreal &brightnessValue);

    ///
    /// \brief Retourne la valeur du contraste
    /// \return La valeur du contraste
    ///
    qreal getContrastValue() const;

    ///
    /// \brief Fixe la valeur du contraste
    /// \param brightnessValue La valeur du contraste
    ///
    void setContrastValue(const qreal &contrastValue);

    /// Exporte les propriétés dans le QSetting passé en paramètre
    /// \param config Le setting dans lequel écrire
    ///
    void exportToFile(QSettings *config);

    /// Importe les paramètres depuis le Qsetting passé en paramètre
    /// \param config Le setting depuis lequel lire les paramètres
    ///
    void importFromFile(QSettings *config);

    cv::Mat rotateCvMAtImage(cv::Mat matOriginal, int angle = 0);
private:

    ///
    /// \brief Convertit une cv::Mat en QImage
    /// \param mat La matrice à convertir
    /// \param format Le QImage::format de l'image
    /// \return La QImage convertie
    ///
    QImage  cvMatToQimage(const cv::Mat &mat, QImage::Format format);

    ///
    /// \brief Convertit une QImage en cv::Mat avec ou sans copie
    /// \param inImage L'image à convertir
    /// \param inCloneImageData Faut-il la copier ?
    /// \return La matrice de l'image
    ///
    cv::Mat qImageToCvMat(const QImage &inImage, bool inCloneImageData = true);

    void updateContrastAndBrightness(const cv::Mat srcMat, cv::Mat desMat, qreal contrastValue, qreal brightnessValue);

    bool         m_localDebug;
    const QImage m_image; /* L'image originale */
    int          m_angleRotation;
    bool         m_flipXaxis;
    bool         m_flipYaxis;
    QSize        m_iconSize;
    QString      m_pathImage;
    qreal        m_brightnessValue;
    qreal        m_contrastValue;



signals:

public slots:

};
#endif // VIZUALIZATORIMAGE_H

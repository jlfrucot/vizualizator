#ifndef VIZUALIZATORIMAGE_H
#define VIZUALIZATORIMAGE_H

#include <QImage>

class VizualizatorImage : public QImage
{
    Q_OBJECT
public:
    explicit VizualizatorImage(QObject *parent = 0);

signals:

public slots:

};

#endif // VIZUALIZATORIMAGE_H

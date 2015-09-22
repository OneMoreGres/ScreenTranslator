#ifndef IMAGEPROCESSING_H
#define IMAGEPROCESSING_H

#include <QImage>

class Pix;

//! Convert QImage to Leptonica's PIX.
Pix * convertImage (const QImage &image);
//! Convert Leptonica's PIX to QImage.
QImage convertImage (Pix &image);

//! Propare image for OCR.
Pix * prepareImage (const QImage &image, int preferredScale);
//! Free allocated resources for image.
void cleanupImage (Pix **image);

#endif // IMAGEPROCESSING_H

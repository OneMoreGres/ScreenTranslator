#include <QDebug>

#include <leptonica/allheaders.h>

#include "ImageProcessing.h"
#include "StAssert.h"

#if defined(Q_OS_LINUX)
#  include <fstream>
#  include <limits>
qint64 getFreeMemory () {
  std::string token;
  std::ifstream file ("/proc/meminfo");
  qint64 freeMem = 0;
  while (file >> token) {
    if (token == "MemFree:" || token == "Buffers:" || token == "Cached:") {
      unsigned long mem = 0;
      freeMem += (file >> mem) ? mem : 0;
    }
  }
  return freeMem * 1024;
}
#elif defined(Q_OS_WIN)
#  include <windows.h>
qint64 getFreeMemory () {
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof (statex);
  if (GlobalMemoryStatusEx (&statex)) {
    return statex.ullAvailPhys;
  }
  return -1;
}
#endif

Pix * convertImage (const QImage &image) {
  PIX *pix;

  int width = image.width ();
  int height = image.height ();
  int depth = image.depth ();
  int bytesPerLine = image.bytesPerLine ();
  int wpl = bytesPerLine / 4;

  pix = pixCreate (width, height, depth);
  pixSetWpl (pix, wpl);
  pixSetColormap (pix, NULL);
  memmove (pix->data, image.bits (), bytesPerLine * height);

  const qreal toDPM = 1.0 / 0.0254;
  int resolutionX = image.dotsPerMeterX () / toDPM;
  int resolutionY = image.dotsPerMeterY () / toDPM;

  if (resolutionX < 300) {
    resolutionX = 300;
  }
  if (resolutionY < 300) {
    resolutionY = 300;
  }
  pixSetResolution (pix, resolutionX, resolutionY);
  return pix;
}

QImage convertImage (Pix &image) {
  int width = pixGetWidth (&image);
  int height = pixGetHeight (&image);
  int depth = pixGetDepth (&image);
  int bytesPerLine = pixGetWpl (&image) * 4;
  l_uint32 *datas = pixGetData (&image);

  QImage::Format format;
  if (depth == 1) {
    format = QImage::Format_Mono;
  }
  else if (depth == 8) {
    format = QImage::Format_Indexed8;
  }
  else {
    format = QImage::Format_RGB32;
  }

  QImage result ((uchar *)datas, width, height, bytesPerLine, format);

  // Set resolution
  l_int32 xres, yres;
  pixGetResolution (&image, &xres, &yres);
  const qreal toDPM = 1.0 / 0.0254;
  result.setDotsPerMeterX (xres * toDPM);
  result.setDotsPerMeterY (yres * toDPM);

  // Handle pallete
  QVector<QRgb> _bwCT;
  _bwCT.append (qRgb (255,255,255));
  _bwCT.append (qRgb (0,0,0));

  QVector<QRgb> _grayscaleCT (256);
  for (int i = 0; i < 256; i++) {
    _grayscaleCT.append (qRgb (i, i, i));
  }
  switch (depth) {
    case 1:
      result.setColorTable (_bwCT);
      break;
    case 8:
      result.setColorTable (_grayscaleCT);
      break;
    default:
      result.setColorTable (_grayscaleCT);
  }

  if (result.isNull ()) {
    static QImage none (0,0,QImage::Format_Invalid);
    qDebug ("Invalid format!!!\n");
    return none;
  }

  return result;
}

Pix * prepareImage (const QImage &image, int preferredScale) {
  Pix *pix = convertImage (image);
  ST_ASSERT (pix != NULL);

  Pix *gray = pixConvertRGBToGray (pix, 0.0, 0.0, 0.0);
  ST_ASSERT (gray != NULL);
  pixDestroy (&pix);

  Pix *scaled = gray;
  if (preferredScale > 0) {
    const auto MAX_INT16 = 0x7fff;
    float maxScaleX = MAX_INT16 / double (gray->w);
    float scaleX = std::min (float (preferredScale), maxScaleX);
    float maxScaleY = MAX_INT16 / double (gray->h);
    float scaleY = std::min (float (preferredScale), maxScaleY);
    float scale = std::min (scaleX, scaleY);

    qint64 availableMemory = getFreeMemory () * 0.95;
    if (availableMemory > 0) {
      qint32 actualSize = gray->w * gray->h * gray->d / 8;
      float maxScaleMemory = float (availableMemory) / actualSize;
      scale = std::min (scale, maxScaleMemory);
    }
    scaled = pixScale (gray, scale, scale);
    if (scaled == NULL) {
      scaled = gray;
    }
  }
  if (scaled != gray) {
    pixDestroy (&gray);
  }
  return scaled;
}

void cleanupImage (Pix **image) {
  pixDestroy (image);
}

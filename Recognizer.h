#ifndef RECOGNIZER_H
#define RECOGNIZER_H

#include <QObject>
#include "QPixmap"

#include "ProcessingItem.h"

namespace tesseract {
  class TessBaseAPI;
}
class RecognizerHelper;

class Recognizer : public QObject {
  Q_OBJECT

  public:
    explicit Recognizer (QObject *parent = 0);

  signals:
    void recognized (ProcessingItem item);
    void error (QString text);

  public slots:
    void recognize (ProcessingItem item);
    void applySettings ();

  private:
    bool initEngine (tesseract::TessBaseAPI * &engine, const QString &language);

  private:
    tesseract::TessBaseAPI *engine_;
    RecognizerHelper *recognizerHelper_;

    QString tessDataDir_;
    QString ocrLanguage_;
    int imageScale_;

};

#endif // RECOGNIZER_H

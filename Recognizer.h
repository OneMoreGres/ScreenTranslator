#ifndef RECOGNIZER_H
#define RECOGNIZER_H

#include <QObject>
#include "QPixmap"


namespace tesseract
{
  class TessBaseAPI;
}

class Recognizer : public QObject
{
    Q_OBJECT
  public:
    explicit Recognizer(QObject *parent = 0);

  signals:
    void recognized (QString text);
    void recognized (QPixmap pixmap, QString text);
    void error (QString text);

  public slots:
    void recognize (QPixmap pixmap);
    void applySettings ();

  private:
    bool initEngine ();

  private:
    tesseract::TessBaseAPI* engine_;

    QString tessDataDir_;
    QString ocrLanguage_;
    int imageScale_;

};

#endif // RECOGNIZER_H

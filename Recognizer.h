#ifndef RECOGNIZER_H
#define RECOGNIZER_H

#include <QObject>
#include "QPixmap"

class Recognizer : public QObject
{
    Q_OBJECT
  public:
    explicit Recognizer(QObject *parent = 0);

  signals:
    void recognized (QString text);

  public slots:
    void recognize (QPixmap pixmap);

};

#endif // RECOGNIZER_H

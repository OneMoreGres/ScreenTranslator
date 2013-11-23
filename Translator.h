#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QObject>

class Translator : public QObject
{
    Q_OBJECT
  public:
    explicit Translator(QObject *parent = 0);

  signals:
    void translated (QString sourceText, QString translatedText);

  public slots:
    void translate (QString text);

};

#endif // TRANSLATOR_H

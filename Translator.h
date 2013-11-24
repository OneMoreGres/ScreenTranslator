#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QNetworkAccessManager>

class Translator : public QObject
{
    Q_OBJECT
  public:
    explicit Translator(QObject *parent = 0);

  signals:
    void translated (QString sourceText, QString translatedText);
    void error (QString text);

  public slots:
    void translate (QString text);
    void applySettings ();

  private slots:
    void replyFinished (QNetworkReply* reply);

  private:
    QNetworkAccessManager network_;
    QString translationLanguage_;

};

#endif // TRANSLATOR_H

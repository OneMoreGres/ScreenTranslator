#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QNetworkAccessManager>

#include "ProcessingItem.h"

class Translator : public QObject
{
    Q_OBJECT
  public:
    explicit Translator(QObject *parent = 0);

  signals:
    void translated (ProcessingItem item);
    void translateAlternative (ProcessingItem item);
    void error (QString text);

  public slots:
    void translate (ProcessingItem item);
    void translatedAlternative (ProcessingItem item, bool success);
    void applySettings ();

  private slots:
    void replyFinished (QNetworkReply* reply);

  private:
    QNetworkAccessManager network_;
    QString translationLanguage_;
    QString sourceLanguage_;
    QHash<QNetworkReply*, ProcessingItem> items_;
    bool useAlternativeTranslation_;

};

#endif // TRANSLATOR_H

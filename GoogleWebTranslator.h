#ifndef GOOGLEWEBTRANSLATOR_H
#define GOOGLEWEBTRANSLATOR_H

#include <QObject>

#include "ProcessingItem.h"

class QWebView;
class QUrl;
class QNetworkReply;

class GoogleWebTranslator : public QObject {
  Q_OBJECT

  public:
    GoogleWebTranslator ();
    ~GoogleWebTranslator ();

  signals:
    void translated (ProcessingItem item, bool success);
    void error (QString text);

  public slots:
    void translate (ProcessingItem item);
    void applySettings ();

  private slots:
    void loadStarted ();
    void loadFinished (bool ok = true);
    void replyFinished (QNetworkReply *reply);

  private:
    void load (const ProcessingItem &item);

  private:
    QVector<ProcessingItem> queue_;
    QString translationLanguage_;
    QWebView *view_;
    bool isLoadFinished_;
    bool isTranslationFinished_;
};

#endif // GOOGLEWEBTRANSLATOR_H

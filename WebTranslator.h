#ifndef WEBTRANSLATOR_H
#define WEBTRANSLATOR_H

#include <QObject>
#include <QMap>
#include <QTimer>

#include "ProcessingItem.h"

class QWebView;
class QNetworkReply;
class QSslError;

class WebTranslatorProxy;
class TranslatorHelper;

class WebTranslator : public QObject {
  Q_OBJECT

  public:
    explicit WebTranslator ();
    ~WebTranslator ();

  signals:
    void translated (ProcessingItem item);
    void error (QString text);

  public slots:
    void translate (ProcessingItem item);
    void applySettings ();
    void setDebugMode (bool isOn);

  private slots:
    void loadFinished (bool ok);
    void replyFinished (QNetworkReply *reply);
    void addProxyToView ();
    void abortTranslation ();
    void proxyTranslated (const QString &text);
    void handleSslErrors (QNetworkReply *reply, const QList<QSslError> &errors);

  private:
    void translateQueued ();
    void finishTranslation (bool markAsTranslated = true);
    bool tryNextTranslator (bool firstTime = false);

  private:
    WebTranslatorProxy *proxy_;
    QWebView *view_;
    TranslatorHelper *translatorHelper_;
    QVector<ProcessingItem> queue_;
    bool isReady_;
    bool ignoreSslErrors_;
    QTimer translationTimeout_;
};

#endif // WEBTRANSLATOR_H

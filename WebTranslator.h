#ifndef WEBTRANSLATOR_H
#define WEBTRANSLATOR_H

#include <QObject>
#include <QMap>
#include <QTimer>

#include "ProcessingItem.h"

class QWebView;
class QNetworkReply;

class WebTranslatorProxy;

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
    void proxyError (const QString &message);
    void proxyTranslated (const QString &text);

  private:
    void translateQueued ();
    void runScriptForItem (const ProcessingItem &item);
    void finishTranslation (bool markAsTranslated = true);

  private:
    WebTranslatorProxy *proxy_;
    QWebView *view_;
    QVector<ProcessingItem> queue_;
    bool isReady_;
    QString script_;
    QTimer translationTimeout_;
};

#endif // WEBTRANSLATOR_H

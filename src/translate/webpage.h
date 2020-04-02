#pragma once

#include "stfwd.h"

#include <QWebEngineCertificateError>
#include <QWebEngineView>

class WebPageProxy;

class WebPage : public QWebEnginePage
{
  Q_OBJECT
public:
  WebPage(Translator &translator, const QString &script,
          const QString &scriptName);
  ~WebPage();

  void setIgnoreSslErrors(bool ignoreSslErrors);
  void setTimeout(std::chrono::seconds timeout);

  void start(const TaskPtr &task);
  void setTranslated(const QString &text);
  void setFailed(const QString &error);
  bool checkBusy();
  TaskPtr task() const;

  bool isLoadImages() const;
  void setLoadImages(bool isOn);

signals:
  void log(const QString &text);

protected:
  void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level,
                                const QString &message, int lineNumber,
                                const QString &sourceID) override;
  bool certificateError(const QWebEngineCertificateError &error) override;

private:
  void authenticateProxy(const QUrl &requestUrl, QAuthenticator *authenticator,
                         const QString &proxyHost);
  void scheduleWebchannelInitScript();
  void scheduleTranslatorScript(const QString &script);
  void addErrorToTask(const QString &text) const;
  void changeUserAgent();

  Translator &translator_;
  QString scriptName_;
  std::unique_ptr<WebPageProxy> proxy_;
  TaskPtr task_;
  bool ignoreSslErrors_{false};
  bool isBusy_{false};
  QDateTime nextIdleTime_;
  std::chrono::seconds timeout_{15};
};

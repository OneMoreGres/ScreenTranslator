#include "webpage.h"
#include "debug.h"
#include "languagecodes.h"
#include "task.h"
#include "translator.h"
#include "webpageproxy.h"

#include <QWebEngineProfile>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>
#include <QtWebChannel>

WebPage::WebPage(Translator &translator, const QString &script,
                 const QString &scriptName)
  : QWebEnginePage(new QWebEngineProfile)
  , translator_(translator)
  , proxy_(new WebPageProxy(*this))
{
  profile()->setParent(this);

  connect(this, &WebPage::proxyAuthenticationRequired, this,
          &WebPage::authenticateProxy);

  scheduleWebchannelInitScript();
  scheduleTranslatorScript(script, scriptName);

  settings()->setAttribute(QWebEngineSettings::AutoLoadImages, false);

  auto channel = new QWebChannel(this);
  channel->registerObject("proxy", proxy_.get());
  setWebChannel(channel, QWebEngineScript::ScriptWorldId::UserWorld);

  // to load scripts
  setUrl(QUrl::fromUserInput("about:blank"));
}

WebPage::~WebPage() = default;

void WebPage::scheduleWebchannelInitScript()
{
  QFile f(":/qtwebchannel/qwebchannel.js");
  if (!f.open(QFile::ReadOnly)) {
    LERROR() << "Failed to open bundled file" << f.fileName();
    return;
  }
  const auto data =
      QString::fromUtf8(f.readAll()) +
      R"(new QWebChannel(qt.webChannelTransport, function(channel){
window.proxy = channel.objects.proxy;
if (typeof init === "function") init ();
      });)";

  QWebEngineScript js;

  js.setSourceCode(data);
  js.setName("qwebchannel.js");
  js.setWorldId(QWebEngineScript::UserWorld);
  js.setInjectionPoint(QWebEngineScript::Deferred);
  js.setRunsOnSubFrames(false);

  SOFT_ASSERT(profile(), return );
  profile()->scripts()->insert(js);
}

void WebPage::scheduleTranslatorScript(const QString &script,
                                       const QString &scriptName)
{
  QWebEngineScript js;

  js.setSourceCode(script);
  js.setName(scriptName);
  js.setWorldId(QWebEngineScript::UserWorld);
  js.setInjectionPoint(QWebEngineScript::Deferred);
  js.setRunsOnSubFrames(false);

  SOFT_ASSERT(profile(), return );
  profile()->scripts()->insert(js);
}

void WebPage::setIgnoreSslErrors(bool ignoreSslErrors)
{
  ignoreSslErrors_ = ignoreSslErrors;
}

void WebPage::setTimeout(std::chrono::seconds timeout)
{
  timeout_ = timeout;
}

void WebPage::start(const TaskPtr &task)
{
  LanguageCodes languages;
  auto langCodes = languages.findById(task->targetLanguage);
  if (!langCodes) {
    task->error = QObject::tr("unknown translation language: %1")
                      .arg(task->targetLanguage);
    translator_.finish(task);
    return;
  }

  task_ = task;
  isBusy_ = true;
  nextIdleTime_ = QDateTime::currentDateTime().addSecs(timeout_.count());

  proxy_->translate(task->recognized, task->sourceLanguage,
                    langCodes->iso639_1);
}

bool WebPage::isBusy() const
{
  return task_ && isBusy_ && QDateTime::currentDateTime() < nextIdleTime_;
}

void WebPage::setTranslated(const QString &text)
{
  if (!isBusy())
    return;

  isBusy_ = false;

  SOFT_ASSERT(task_, return )
  task_->translated = text;
  translator_.finish(task_);
}

void WebPage::setFailed(const QString &error)
{
  if (!isBusy())
    return;

  isBusy_ = false;

  SOFT_ASSERT(task_, return )
  //  task_->error = error;
  translator_.finish(task_);
}

TaskPtr WebPage::task() const
{
  return task_;
}

void WebPage::javaScriptConsoleMessage(
    QWebEnginePage::JavaScriptConsoleMessageLevel /*level*/,
    const QString &message, int lineNumber, const QString &sourceID)
{
  qDebug() << sourceID << ":" << lineNumber << message;
  emit log(QString("%1: %2 %3").arg(sourceID).arg(lineNumber).arg(message));
}

bool WebPage::certificateError(const QWebEngineCertificateError &error)
{
  qDebug() << "certificateError" << error.url() << error.error()
           << error.errorDescription();
  return ignoreSslErrors_;
}

void WebPage::authenticateProxy(const QUrl & /*requestUrl*/,
                                QAuthenticator *authenticator,
                                const QString & /*proxyHost*/)
{
  const auto proxy = QNetworkProxy::applicationProxy();
  authenticator->setUser(proxy.user());
  authenticator->setPassword(proxy.password());
}

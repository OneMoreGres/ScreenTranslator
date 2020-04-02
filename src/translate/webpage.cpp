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
  , scriptName_(scriptName)
  , proxy_(new WebPageProxy(*this))
{
  profile()->setParent(this);

  changeUserAgent();

  connect(this, &WebPage::proxyAuthenticationRequired, this,
          &WebPage::authenticateProxy);

  scheduleWebchannelInitScript();
  scheduleTranslatorScript(script);

  setLoadImages(false);

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

void WebPage::scheduleTranslatorScript(const QString &script)
{
  QWebEngineScript js;

  js.setSourceCode(script);
  js.setName(scriptName_);
  js.setWorldId(QWebEngineScript::UserWorld);
  js.setInjectionPoint(QWebEngineScript::Deferred);
  js.setRunsOnSubFrames(false);

  SOFT_ASSERT(profile(), return );
  profile()->scripts()->insert(js);
}

void WebPage::addErrorToTask(const QString &text) const
{
  if (!task_)
    return;
  task_->translatorErrors.append(QString("%1: %2").arg(scriptName_, text));
}

void WebPage::changeUserAgent()
{
  auto userAgent = profile()->httpUserAgent().split(' ');
  userAgent.erase(std::remove_if(userAgent.begin(), userAgent.end(),
                                 [](const QString &part) {
                                   return part.startsWith("QtWebEngine");
                                 }),
                  userAgent.end());
  profile()->setHttpUserAgent(userAgent.join(' '));
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
  const auto sourceLanguage = LanguageCodes::iso639_1(task->sourceLanguage);
  const auto targetLanguage = LanguageCodes::iso639_1(task->targetLanguage);
  if (sourceLanguage.isEmpty() || targetLanguage.isEmpty()) {
    task->error = QObject::tr("unknown translation languages: %1 or %2")
                      .arg(task->sourceLanguage)
                      .arg(task->targetLanguage);
    translator_.finish(task);
    return;
  }

  task_ = task;
  isBusy_ = true;
  nextIdleTime_ = QDateTime::currentDateTime().addSecs(timeout_.count());

  proxy_->translate(task->corrected, sourceLanguage, targetLanguage);
}

bool WebPage::checkBusy()
{
  if (!task_ || !isBusy_)
    return false;

  if (QDateTime::currentDateTime() < nextIdleTime_)
    return true;

  addErrorToTask(tr("timed out"));
  isBusy_ = false;

  return false;
}

void WebPage::setTranslated(const QString &text)
{
  if (!checkBusy())
    return;

  isBusy_ = false;

  SOFT_ASSERT(task_, return )
  task_->translated = text;
  translator_.finish(task_);
}

void WebPage::setFailed(const QString &error)
{
  if (!checkBusy())
    return;

  isBusy_ = false;

  addErrorToTask(error);
}

TaskPtr WebPage::task() const
{
  return task_;
}

bool WebPage::isLoadImages() const
{
  return settings()->testAttribute(QWebEngineSettings::AutoLoadImages);
}

void WebPage::setLoadImages(bool isOn)
{
  settings()->setAttribute(QWebEngineSettings::AutoLoadImages, isOn);
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

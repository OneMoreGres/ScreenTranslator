#include "manager.h"
#include "capturer.h"
#include "corrector.h"
#include "debug.h"
#include "recognizer.h"
#include "representer.h"
#include "settingseditor.h"
#include "task.h"
#include "translator.h"
#include "trayicon.h"
#include "updates.h"

#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QNetworkProxy>

namespace
{
#ifdef DEVELOP
const auto updatesUrl = "http://localhost:8081/updates.json";
#else
const auto updatesUrl =
    "https://raw.githubusercontent.com/OneMoreGres/ScreenTranslator/master/"
    "updates.json";
#endif
}  // namespace

Manager::Manager()
  : updater_(std::make_unique<update::Loader>(QUrl(updatesUrl)))
{
  tray_ = std::make_unique<TrayIcon>(*this);
  capturer_ = std::make_unique<Capturer>(*this);
  recognizer_ = std::make_unique<Recognizer>(*this);
  translator_ = std::make_unique<Translator>(*this);
  corrector_ = std::make_unique<Corrector>(*this);
  representer_ = std::make_unique<Representer>(*this, *tray_);

  qRegisterMetaType<TaskPtr>();

  Settings settings;
  settings.load();
  updateSettings(settings);

  if (settings.showMessageOnStart)
    tray_->showInformation(QObject::tr("Screen translator started"));

  QObject::connect(updater_.get(), &update::Loader::error,  //
                   tray_.get(), &TrayIcon::showError);
  QObject::connect(updater_.get(), &update::Loader::updated,  //
                   tray_.get(), [this] {
                     tray_->showInformation(QObject::tr("Update completed"));
                   });
  QObject::connect(updater_.get(), &update::Loader::updatesAvailable,  //
                   tray_.get(), [this] {
                     tray_->showInformation(QObject::tr("Updates available"));
                   });
#ifdef DEVELOP
  updater_->checkForUpdates();
#endif
}

Manager::~Manager() = default;

void Manager::updateSettings(const Settings &settings)
{
  LTRACE() << "updateSettings";
  setupProxy(settings);

  updater_->model()->setExpansions({
      {"$translators$", settings.translatorsDir},
      {"$tessdata$", settings.tessdataPath},
  });

  tray_->updateSettings(settings);
  capturer_->updateSettings(settings);
  recognizer_->updateSettings(settings);
  translator_->updateSettings(settings);
  corrector_->updateSettings(settings);
  representer_->updateSettings(settings);
}

void Manager::setupProxy(const Settings &settings)
{
  if (settings.proxyType == ProxyType::System) {
    QNetworkProxyFactory::setUseSystemConfiguration(true);
    return;
  }

  QNetworkProxyFactory::setUseSystemConfiguration(false);

  if (settings.proxyType == ProxyType::Disabled) {
    QNetworkProxy::setApplicationProxy({});
    return;
  }

  QNetworkProxy proxy;
  using T = QNetworkProxy::ProxyType;
  proxy.setType(settings.proxyType == ProxyType::Socks5 ? T::Socks5Proxy
                                                        : T::HttpProxy);
  proxy.setHostName(settings.proxyHostName);
  proxy.setPort(settings.proxyPort);
  proxy.setUser(settings.proxyUser);
  proxy.setPassword(settings.proxyPassword);
  QNetworkProxy::setApplicationProxy(proxy);
}

void Manager::finishTask(const TaskPtr &task)
{
  SOFT_ASSERT(task, return );
  LTRACE() << "finishTask" << task->captured << task->error;

  --activeTaskCount_;
  tray_->setActiveTaskCount(activeTaskCount_);

  last_ = task;
  tray_->setTaskActionsEnabled(last_->isValid());

  if (!task->isValid()) {
    tray_->showError(task->error);
    return;
  }

  tray_->showSuccess();
}

void Manager::captured(const TaskPtr &task)
{
  tray_->blockActions(false);

  SOFT_ASSERT(task, return );
  LTRACE() << "captured" << task->captured << task->error;

  ++activeTaskCount_;
  tray_->setActiveTaskCount(activeTaskCount_);

  if (!task->isValid()) {
    finishTask(task);
    return;
  }

  recognizer_->recognize(task);
}

void Manager::captureCanceled()
{
  tray_->blockActions(false);
}

void Manager::recognized(const TaskPtr &task)
{
  SOFT_ASSERT(task, return );
  LTRACE() << "recognized" << task->recognized << task->error;

  if (!task->isValid()) {
    finishTask(task);
    return;
  }

  corrector_->correct(task);
}

void Manager::corrected(const TaskPtr &task)
{
  SOFT_ASSERT(task, return );
  LTRACE() << "corrected" << task->recognized << task->error;

  if (!task->isValid()) {
    finishTask(task);
    return;
  }

  if (!task->targetLanguage.isEmpty())
    translator_->translate(task);
  else
    translated(task);
}

void Manager::translated(const TaskPtr &task)
{
  SOFT_ASSERT(task, return );
  LTRACE() << "translated" << task->recognized << task->error;

  finishTask(task);

  if (!task->isValid())
    return;

  representer_->represent(task);
}

void Manager::applySettings(const Settings &settings)
{
  updateSettings(settings);
  settings.save();
}

void Manager::fatalError(const QString &text)
{
  tray_->blockActions(false);
  tray_->showFatalError(text);
}

void Manager::capture()
{
  SOFT_ASSERT(capturer_, return );
  tray_->blockActions(true);
  capturer_->capture();
  tray_->setRepeatCaptureEnabled(true);
}

void Manager::repeatCapture()
{
  SOFT_ASSERT(capturer_, return );
  tray_->blockActions(true);
  capturer_->repeatCapture();
}

void Manager::showLast()
{
  if (!last_ || !last_->isValid())
    return;
  SOFT_ASSERT(representer_, return );
  representer_->represent(last_);
}

void Manager::settings()
{
  SettingsEditor editor(*this, *updater_);

  Settings settings;
  settings.load();
  editor.setSettings(settings);

  tray_->blockActions(true);
  auto result = editor.exec();
  tray_->blockActions(false);

  if (result != QDialog::Accepted)
    return;

  tray_->resetFatalError();

  settings = editor.settings();
  settings.save();
  updateSettings(settings);
}

void Manager::copyLastToClipboard()
{
  if (!last_ || !last_->isValid())
    return;

  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setText(last_->recognized + QLatin1String(" - ") +
                     last_->translated);
  tray_->showInformation(
      QObject::tr("The last result was copied to the clipboard."));
}

void Manager::about()
{
  auto text =
      QObject::tr(R"(Optical character recognition (OCR) and translation tool
Author: Gres (translator@gres.biz)
Version: %1)")
          .arg(QApplication::applicationVersion());

  QMessageBox message(QMessageBox::Information, QObject::tr("About"), text,
                      QMessageBox::Ok);
  message.setIconPixmap(QIcon(":/icons/app.png").pixmap(QSize(64, 64)));
  message.exec();
}

void Manager::quit()
{
  QApplication::quit();
}

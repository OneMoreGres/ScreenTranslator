#include "manager.h"
#include "capturer.h"
#include "commonmodels.h"
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
  : settings_(std::make_unique<Settings>())
  , updater_(std::make_unique<update::Loader>(QUrl(updatesUrl)))
  , models_(std::make_unique<CommonModels>())
{
  SOFT_ASSERT(settings_, return );

  tray_ = std::make_unique<TrayIcon>(*this, *settings_);
  capturer_ = std::make_unique<Capturer>(*this, *settings_, *models_);
  recognizer_ = std::make_unique<Recognizer>(*this, *settings_);
  translator_ = std::make_unique<Translator>(*this, *settings_);
  corrector_ = std::make_unique<Corrector>(*this, *settings_);
  representer_ =
      std::make_unique<Representer>(*this, *tray_, *settings_, *models_);
  qRegisterMetaType<TaskPtr>();

  settings_->load();
  updateSettings();

  if (settings_->showMessageOnStart)
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
}

Manager::~Manager()
{
  SOFT_ASSERT(settings_, return );
  if (updateAutoChecker_ && updateAutoChecker_->isLastCheckDateChanged()) {
    settings_->lastUpdateCheck = updateAutoChecker_->lastCheckDate();
    settings_->saveLastUpdateCheck();
  }
}

void Manager::updateSettings()
{
  LTRACE() << "updateSettings";
  SOFT_ASSERT(settings_, return );
  setupProxy(*settings_);
  setupUpdates(*settings_);

  models_->update(settings_->tessdataPath);

  tray_->updateSettings();
  capturer_->updateSettings();
  recognizer_->updateSettings();
  translator_->updateSettings();
  corrector_->updateSettings();
  representer_->updateSettings();
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

void Manager::setupUpdates(const Settings &settings)
{
  updater_->model()->setExpansions({
      {"$translators$", settings.translatorsDir},
      {"$tessdata$", settings.tessdataPath},
  });

  if (settings.autoUpdateIntervalDays > 0) {
    updateAutoChecker_ = std::make_unique<update::AutoChecker>(*updater_);
    updateAutoChecker_->setLastCheckDate(settings.lastUpdateCheck);
    updateAutoChecker_->setCheckIntervalDays(settings.autoUpdateIntervalDays);
  } else {
    updateAutoChecker_.reset();
  }
}

void Manager::finishTask(const TaskPtr &task)
{
  SOFT_ASSERT(task, return );
  LTRACE() << "finishTask" << task->captured << task->error;

  --activeTaskCount_;
  tray_->setActiveTaskCount(activeTaskCount_);

  if (!task->isValid()) {
    tray_->showError(task->error);
    tray_->setTaskActionsEnabled(false);
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

  representer_->represent(task);
  tray_->setTaskActionsEnabled(!task->isNull());
}

void Manager::applySettings(const Settings &settings)
{
  SOFT_ASSERT(settings_, return );
  *settings_ = settings;
  settings_->save();
  updateSettings();
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

void Manager::settings()
{
  SettingsEditor editor(*this, *updater_);

  SOFT_ASSERT(settings_, return );
  editor.setSettings(*settings_);

  tray_->blockActions(true);
  auto result = editor.exec();
  tray_->blockActions(false);

  if (result != QDialog::Accepted)
    return;

  tray_->resetFatalError();

  const auto edited = editor.settings();
  applySettings(edited);
}

void Manager::showLast()
{
  SOFT_ASSERT(representer_, return );
  representer_->showLast();
}

void Manager::copyLastToClipboard()
{
  SOFT_ASSERT(representer_, return );
  representer_->clipboardLast();
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

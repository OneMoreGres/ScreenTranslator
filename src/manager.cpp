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

#include <QApplication>
#include <QClipboard>
#include <QMessageBox>

Manager::Manager()
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
}

Manager::~Manager() = default;

void Manager::updateSettings(const Settings &settings)
{
  LTRACE() << "updateSettings";
  tray_->updateSettings(settings);
  capturer_->updateSettings(settings);
  recognizer_->updateSettings(settings);
  translator_->updateSettings(settings);
  corrector_->updateSettings(settings);
  representer_->updateSettings(settings);
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
  SettingsEditor editor;

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

#include "capturer.h"
#include "captureareaselector.h"
#include "manager.h"
#include "settings.h"
#include "task.h"

#include <QApplication>

Capturer::Capturer(Manager &manager, const Settings &settings)
  : manager_(manager)
  , settings_(settings)
{
}

void Capturer::capture()
{
  showOverlays(true);
}

void Capturer::repeatCapture()
{
  showOverlays(false);
}

void Capturer::updateSettings()
{
}

void Capturer::captured(const TaskPtr &task)
{
  hideOverlays();
  // TODO respect more overlay's options
  // TODO process modifiers
  task->translators = settings_.translators;
  task->sourceLanguage = settings_.sourceLanguage;
  if (settings_.doTranslation)
    task->targetLanguage = settings_.targetLanguage;
  manager_.captured(task);
}

void Capturer::canceled()
{
  hideOverlays();
  manager_.captureCanceled();
}

void Capturer::showOverlays(bool capturePixmap)
{
  const auto screens = QApplication::screens();
  const auto screensSize = screens.size();
  int overlaysSize = selectors_.size();
  if (screensSize > overlaysSize)
    selectors_.reserve(screensSize);

  for (auto i = 0, end = screensSize; i < end; ++i) {
    if (i == overlaysSize) {
      selectors_.push_back(new CaptureAreaSelector(*this, settings_));
      ++overlaysSize;
    }

    const auto screen = screens[i];
    auto &overlay = selectors_[i];
    overlay->hide();
    if (capturePixmap)
      overlay->setScreen(*screen);
    overlay->show();
    overlay->activateWindow();
  }

  if (screensSize < overlaysSize) {
    for (auto i = overlaysSize - 1; i >= screensSize; --i)
      selectors_[i]->deleteLater();
  }
}

void Capturer::hideOverlays()
{
  for (const auto &overlay : selectors_) overlay->hide();
}

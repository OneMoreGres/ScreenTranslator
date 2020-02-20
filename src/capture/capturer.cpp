#include "capturer.h"
#include "captureoverlay.h"
#include "manager.h"
#include "settings.h"
#include "task.h"

#include <QApplication>

Capturer::Capturer(Manager &manager)
  : manager_(manager)
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

void Capturer::updateSettings(const Settings &settings)
{
  sourceLanguage_ = settings.sourceLanguage;
  targetLanguage_ = settings.targetLanguage;
  translators_ = settings.translators;
}

void Capturer::captured(const TaskPtr &task)
{
  hideOverlays();
  // TODO respect more overlay's options
  // TODO process modifiers
  task->translators = translators_;
  task->sourceLanguage = sourceLanguage_;
  task->targetLanguage = targetLanguage_;
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
  int overlaysSize = overlays_.size();
  if (screensSize > overlaysSize)
    overlays_.reserve(screensSize);

  for (auto i = 0, end = screensSize; i < end; ++i) {
    if (i == overlaysSize) {
      overlays_.push_back(new CaptureOverlay(*this));
      ++overlaysSize;
    }

    const auto screen = screens[i];
    auto &overlay = overlays_[i];
    overlay->hide();
    if (capturePixmap)
      overlay->setScreen(*screen);
    overlay->show();
    overlay->activateWindow();
  }

  if (screensSize < overlaysSize) {
    for (auto i = overlaysSize - 1; i >= screensSize; --i)
      overlays_[i]->deleteLater();
  }
}

void Capturer::hideOverlays()
{
  for (const auto &overlay : overlays_) overlay->hide();
}

#include "capturer.h"
#include "captureareaselector.h"
#include "debug.h"
#include "manager.h"
#include "settings.h"
#include "task.h"

#include <QApplication>
#include <QPainter>
#include <QScreen>

Capturer::Capturer(Manager &manager, const Settings &settings)
  : manager_(manager)
  , settings_(settings)
  , selector_(std::make_unique<CaptureAreaSelector>(*this, settings_))
{
}

Capturer::~Capturer() = default;

void Capturer::capture()
{
  updatePixmap();
  SOFT_ASSERT(selector_, return );
  selector_->activate();
}

void Capturer::updatePixmap()
{
  const auto screens = QApplication::screens();
  std::vector<QRect> screenRects;
  screenRects.reserve(screens.size());
  QRect rect;

  for (const QScreen *screen : screens) {
    const auto geometry = screen->geometry();
    screenRects.push_back(geometry);
    rect |= geometry;
  }

  QPixmap combined(rect.size());
  QPainter p(&combined);

  for (const auto screen : screens) {
    const auto geometry = screen->geometry();
    const auto pixmap =
        screen->grabWindow(0, 0, 0, geometry.width(), geometry.height());
    p.drawPixmap(geometry, pixmap);
  }

  SOFT_ASSERT(selector_, return );
  selector_->setPixmap(combined);
  selector_->setScreenRects(screenRects);
}

void Capturer::repeatCapture()
{
  SOFT_ASSERT(selector_, return );
  selector_->activate();
}

void Capturer::updateSettings()
{
  SOFT_ASSERT(selector_, return );
  selector_->updateSettings();
}

void Capturer::captured(const TaskPtr &task)
{
  SOFT_ASSERT(selector_, return );
  selector_->hide();

  task->translators = settings_.translators;
  task->sourceLanguage = settings_.sourceLanguage;
  if (settings_.doTranslation)
    task->targetLanguage = settings_.targetLanguage;
  manager_.captured(task);
}

void Capturer::canceled()
{
  SOFT_ASSERT(selector_, return );
  selector_->hide();
  manager_.captureCanceled();
}

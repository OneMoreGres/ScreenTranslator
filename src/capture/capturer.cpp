#include "capturer.h"
#include "capturearea.h"
#include "captureareaselector.h"
#include "debug.h"
#include "manager.h"
#include "settings.h"
#include "waylandcapturer.h"

#include <QApplication>
#include <QPainter>
#include <QScreen>

Capturer::Capturer(Manager &manager, const Settings &settings,
                   const CommonModels &models)
  : manager_(manager)
  , settings_(settings)
  , selector_(std::make_unique<CaptureAreaSelector>(*this, settings_, models,
                                                    pixmap_))
  , wayland_(WaylandCapturer::create())
{
}

Capturer::~Capturer() = default;

void Capturer::capture()
{
  updatePixmap();
  SOFT_ASSERT(selector_, return );
  selector_->activate();
}

bool Capturer::canCaptureLocked()
{
  SOFT_ASSERT(selector_, return false);
  return selector_->hasLocked();
}

void Capturer::captureLocked()
{
  updatePixmap();
  SOFT_ASSERT(selector_, return );
  selector_->captureLocked();
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

  if (!wayland_) {
    for (const auto screen : screens) {
      const auto geometry = screen->geometry();
      const auto pixmap =
          screen->grabWindow(0, 0, 0, geometry.width(), geometry.height());
      p.drawPixmap(geometry, pixmap);
    }
  } else {
    auto pix = wayland_->grab();
    if (!pix.isNull()) {
      p.drawPixmap(0, 0, pix);
    } else {
      combined.fill(Qt::black);
    }
  }

  SOFT_ASSERT(selector_, return );
  pixmap_ = combined;
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

void Capturer::selected(const CaptureArea &area)
{
  SOFT_ASSERT(selector_, return manager_.captureCanceled())
  selector_->hide();

  SOFT_ASSERT(!pixmap_.isNull(), return manager_.captureCanceled())
  auto task = area.task(pixmap_);
  if (task)
    manager_.captured(task);
  else
    manager_.captureCanceled();
}

void Capturer::canceled()
{
  SOFT_ASSERT(selector_, return );
  selector_->hide();
  manager_.captureCanceled();
}

#include "captureareaselector.h"
#include "capturearea.h"
#include "captureareaeditor.h"
#include "capturer.h"
#include "debug.h"
#include "geometryutils.h"
#include "settings.h"

#include <QMenu>
#include <QMouseEvent>
#include <QPainter>

static bool locked(const std::shared_ptr<CaptureArea> &area)
{
  return area->isLocked();
}
static bool notLocked(const std::shared_ptr<CaptureArea> &area)
{
  return !area->isLocked();
}

CaptureAreaSelector::CaptureAreaSelector(Capturer &capturer,
                                         const Settings &settings,
                                         const CommonModels &models,
                                         const QPixmap &pixmap,
                                         const QPoint &pixmapOffset)
  : capturer_(capturer)
  , settings_(settings)
  , pixmap_(pixmap)
  , pixmapOffset_(pixmapOffset)
  , editor_(std::make_unique<CaptureAreaEditor>(models, this))
  , contextMenu_(new QMenu(this))
{
  setWindowFlags(Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint |
                 Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
  setMouseTracking(true);
  setAttribute(Qt::WA_OpaquePaintEvent);

  help_ = tr(R"(Right click on selection - customize
Left click on selection - process
Enter - process all selections
Esc - cancel
Ctrl - keep selecting)");

  {
    auto action = contextMenu_->addAction(tr("Capture all"));
    connect(action, &QAction::triggered,  //
            this, &CaptureAreaSelector::captureAll);
  }
  {
    auto action = contextMenu_->addAction(tr("Cancel"));
    connect(action, &QAction::triggered,  //
            this, &CaptureAreaSelector::cancel);
  }
}

CaptureAreaSelector::~CaptureAreaSelector() = default;

void CaptureAreaSelector::activate()
{
  setGeometry(QRect(pixmapOffset_, pixmap_.size()));
  show();
  activateWindow();
}

bool CaptureAreaSelector::hasLocked() const
{
  const auto it = std::find_if(areas_.cbegin(), areas_.cend(), locked);
  return it != areas_.cend();
}

void CaptureAreaSelector::captureLocked()
{
  SOFT_ASSERT(hasLocked(), return );
  ++generation_;
  for (auto &area : areas_) {
    if (area->isLocked())
      capture(*area, generation_);
  }
}

void CaptureAreaSelector::capture(CaptureArea &area, uint generation)
{
  area.setGeneration(generation);
  capturer_.selected(area);
}

void CaptureAreaSelector::captureAll()
{
  SOFT_ASSERT(!areas_.empty(), return );
  ++generation_;
  for (auto &area : areas_) capture(*area, generation_);
}

void CaptureAreaSelector::cancel()
{
  capturer_.canceled();
}

void CaptureAreaSelector::updateCursorShape(const QPoint &pos)
{
  const auto set = [this](Qt::CursorShape shape) {
    const auto current = cursor().shape();
    if (current != shape)
      setCursor(shape);
  };

  if (areas_.empty()) {
    set(Qt::CrossCursor);
    return;
  }

  for (const auto &area : areas_) {
    if (area->rect().contains(pos)) {
      set(Qt::CursorShape::PointingHandCursor);
      return;
    }
  }

  set(Qt::CrossCursor);
}

void CaptureAreaSelector::setScreenRects(const std::vector<QRect> &screens)
{
  auto helpRect = fontMetrics().boundingRect({}, 0, help_);
  helpRect.setSize(helpRect.size() * 1.4);

  helpRects_.clear();
  helpRects_.reserve(screens.size());
  for (const auto &screen : screens) {
    auto possible = std::vector<QRect>(2, helpRect);
    possible[0].moveTopLeft(screen.topLeft());
    possible[1].moveTopRight(screen.topRight());
    helpRects_.push_back({possible[0], possible});
  }
}

void CaptureAreaSelector::updateSettings()
{
  areas_.clear();
}

void CaptureAreaSelector::paintEvent(QPaintEvent * /*event*/)
{
  QPainter painter(this);
  painter.drawPixmap(rect(), pixmap_);

  for (const auto &rect : helpRects_) drawHelpRects(painter, rect);

  if (!areas_.empty()) {
    for (const auto &area : areas_) drawCaptureArea(painter, *area);
  }

  if (editor_->isVisible()) {
    painter.setBrush(QBrush(QColor(200, 200, 200, 200)));
    painter.setPen(Qt::NoPen);
    painter.drawRect(editor_->geometry());
  }

  const auto area = CaptureArea(
      QRect(startSelectPos_, currentSelectPos_).normalized(), settings_);
  if (!area.isValid())
    return;
  drawCaptureArea(painter, area);
}

bool CaptureAreaSelector::updateCurrentHelpRects()
{
  const auto cursor = mapFromGlobal(QCursor::pos());
  auto changed = false;

  for (auto &screenHelp : helpRects_) {
    if (!screenHelp.current.contains(cursor))
      continue;

    for (const auto &screenPossible : screenHelp.possible) {
      if (screenPossible.contains(cursor))
        continue;

      screenHelp.current = screenPossible;
      changed = true;
      break;
    }
  }

  return changed;
}

void CaptureAreaSelector::drawHelpRects(QPainter &painter,
                                        const HelpRect &rect) const
{
  painter.setBrush(QBrush(QColor(200, 200, 200, 200)));
  painter.setPen(Qt::NoPen);
  painter.drawRect(rect.current);

  painter.setBrush({});
  painter.setPen(Qt::black);
  painter.drawText(rect.current, Qt::AlignCenter, help_);
}

void CaptureAreaSelector::drawCaptureArea(QPainter &painter,
                                          const CaptureArea &area) const
{
  const auto areaRect = area.rect();
  const auto toolTip = area.toolTip();
  auto toolTipRect = painter.boundingRect(QRect(), 0, toolTip);
  toolTipRect.moveTopLeft(areaRect.topLeft() - QPoint(0, toolTipRect.height()));

  painter.setBrush(QBrush(QColor(200, 200, 200, 50)));
  painter.setPen(Qt::NoPen);
  painter.drawRect(areaRect);

  painter.setBrush(QBrush(QColor(200, 200, 200, 200)));
  painter.drawRect(toolTipRect);

  painter.setBrush({});
  painter.setPen(Qt::red);
  painter.drawRect(areaRect);

  painter.setPen(Qt::black);
  painter.drawText(toolTipRect, 0, toolTip);
}

void CaptureAreaSelector::showEvent(QShowEvent * /*event*/)
{
  editor_->hide();
  startSelectPos_ = currentSelectPos_ = QPoint();
  areas_.erase(std::remove_if(areas_.begin(), areas_.end(), notLocked),
               areas_.end());
  updateCursorShape(QCursor::pos());
}

void CaptureAreaSelector::hideEvent(QHideEvent * /*event*/)
{
  editor_->hide();
}

void CaptureAreaSelector::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Escape) {
    if (editor_ && editor_->isVisible())
      applyEditor();
    cancel();
    return;
  }

  if (event->key() == Qt::Key_Return) {
    if (editor_ && editor_->isVisible())
      applyEditor();

    if (!areas_.empty()) {
      captureAll();
    } else {
      cancel();
    }
    return;
  }
}

void CaptureAreaSelector::mousePressEvent(QMouseEvent *event)
{
  SOFT_ASSERT(editor_, return );
  if (editor_->isVisible()) {
    if (editor_->geometry().contains(event->pos()))
      return;
    applyEditor();
  }

  if (!areas_.empty()) {
    for (auto &area : areas_) {
      if (!area->rect().contains(event->pos()))
        continue;

      if (event->button() == Qt::LeftButton) {
        capture(*area, ++generation_);
      } else if (event->button() == Qt::RightButton) {
        customize(area);
      }
      return;
    }
  }

  if (startSelectPos_.isNull())
    startSelectPos_ = currentSelectPos_ = event->pos();
}

void CaptureAreaSelector::mouseMoveEvent(QMouseEvent *event)
{
  updateCursorShape(QCursor::pos());

  if (startSelectPos_.isNull()) {
    if (updateCurrentHelpRects())
      update();
    return;
  }

  currentSelectPos_ = event->pos();
  updateCurrentHelpRects();
  update();
}

void CaptureAreaSelector::mouseReleaseEvent(QMouseEvent *event)
{
  if (startSelectPos_.isNull())
    return;

  const auto endPos = event->pos();
  const auto selection = QRect(startSelectPos_, endPos).normalized();

  startSelectPos_ = currentSelectPos_ = {};

  auto area = CaptureArea(selection, settings_);
  if (!area.isValid()) {  // just a click
    if (areas_.empty()) {
      cancel();
      return;
    }
    if (event->button() == Qt::RightButton) {
      contextMenu_->popup(QCursor::pos());
    }
    return;
  }

  areas_.emplace_back(std::make_unique<CaptureArea>(area));
  if (event->button() == Qt::RightButton) {
    customize(areas_.back());
    return;
  }

  if (!(event->modifiers() & Qt::ControlModifier))
    captureAll();
}

void CaptureAreaSelector::customize(const std::shared_ptr<CaptureArea> &area)
{
  SOFT_ASSERT(editor_, return );
  SOFT_ASSERT(area, return );
  editor_->set(*area);
  edited_ = area;
  editor_->show();
  const auto topLeft = service::geometry::cornerAtPoint(
      area->rect().center(), editor_->size(), geometry());
  editor_->move(topLeft);
  update();
}

void CaptureAreaSelector::applyEditor()
{
  SOFT_ASSERT(editor_, return );
  if (!editor_->isVisible() || edited_.expired())
    return;
  editor_->apply(*edited_.lock());
  editor_->hide();
  update();
}

#include "captureareaselector.h"
#include "capturearea.h"
#include "captureareaeditor.h"
#include "capturer.h"
#include "debug.h"
#include "geometryutils.h"
#include "languagecodes.h"
#include "settings.h"

#include <QMouseEvent>
#include <QPainter>

CaptureAreaSelector::CaptureAreaSelector(Capturer &capturer,
                                         const Settings &settings,
                                         const CommonModels &models,
                                         const QPixmap &pixmap)
  : capturer_(capturer)
  , settings_(settings)
  , pixmap_(pixmap)
  , editor_(std::make_unique<CaptureAreaEditor>(models, this))
{
  setWindowFlags(Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint |
                 Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
  setCursor(Qt::CrossCursor);
  setMouseTracking(true);
  setAttribute(Qt::WA_OpaquePaintEvent);
}

CaptureAreaSelector::~CaptureAreaSelector() = default;

void CaptureAreaSelector::activate()
{
  setGeometry(pixmap_.rect());
  show();
  activateWindow();
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
  const auto sourceName = LanguageCodes::name(settings_.sourceLanguage);
  const auto targetName = LanguageCodes::name(settings_.targetLanguage);

  const auto translationState = settings_.doTranslation ? tr("on") : tr("off");

  help_ = tr(R"(Recognition language: %1
Translation language: %2 (%3)
Right click on selection - customize
Left click on selection - process)")
              .arg(sourceName, targetName, translationState);

  area_.reset();
}

void CaptureAreaSelector::paintEvent(QPaintEvent * /*event*/)
{
  QPainter painter(this);
  painter.drawPixmap(rect(), pixmap_);

  for (const auto &rect : helpRects_) drawHelpRects(painter, rect);

  if (area_)
    drawCaptureArea(painter, *area_);

  if (editor_->isVisible()) {
    painter.setBrush(QBrush(QColor(200, 200, 200, 200)));
    painter.setPen(Qt::NoPen);
    painter.drawRect(editor_->geometry());
  }

  auto selection = QRect(startSelectPos_, currentSelectPos_).normalized();
  if (!selection.isValid())
    return;

  const auto area = CaptureArea(selection, settings_);
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

  painter.setBrush(QBrush(QColor(200, 200, 200, 150)));
  painter.drawRect(toolTipRect);

  painter.setBrush({});
  painter.setPen(Qt::red);
  painter.drawRect(areaRect);

  painter.setPen(Qt::white);
  painter.drawText(toolTipRect, 0, toolTip);
}

void CaptureAreaSelector::showEvent(QShowEvent * /*event*/)
{
  editor_->hide();
  area_.reset();
  startSelectPos_ = currentSelectPos_ = QPoint();
}

void CaptureAreaSelector::hideEvent(QHideEvent * /*event*/)
{
  editor_->hide();
}

void CaptureAreaSelector::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Escape)
    capturer_.canceled();
}

void CaptureAreaSelector::mousePressEvent(QMouseEvent *event)
{
  SOFT_ASSERT(editor_, return );
  if (editor_->isVisible()) {
    if (editor_->geometry().contains(event->pos()))
      return;
    applyEditor();
  }

  if (area_ && area_->rect().contains(event->pos())) {
    if (event->button() == Qt::LeftButton) {
      capturer_.selected(*area_);
    } else if (event->button() == Qt::RightButton) {
      customize(*area_);
    }
    return;
  }

  if (startSelectPos_.isNull())
    startSelectPos_ = event->pos();
}

void CaptureAreaSelector::mouseMoveEvent(QMouseEvent *event)
{
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

  const auto area = CaptureArea(selection, settings_);
  if (!area.isValid()) {
    capturer_.canceled();
    return;
  }

  if (event->button() != Qt::RightButton) {
    capturer_.selected(area);
  } else {
    area_ = std::make_unique<CaptureArea>(area);
    customize(*area_);
  }
}

void CaptureAreaSelector::customize(const CaptureArea &area)
{
  SOFT_ASSERT(editor_, return );
  editor_->set(area);
  editor_->show();
  const auto topLeft = service::geometry::cornerAtPoint(
      area.rect().center(), editor_->size(), geometry());
  editor_->move(topLeft);
  update();
}

void CaptureAreaSelector::applyEditor()
{
  SOFT_ASSERT(editor_, return );
  if (!editor_->isVisible() || !area_)
    return;
  editor_->apply(*area_);
  editor_->hide();
}

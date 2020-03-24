#include "captureareaselector.h"
#include "capturearea.h"
#include "capturer.h"
#include "languagecodes.h"
#include "settings.h"

#include <QMouseEvent>
#include <QPainter>

CaptureAreaSelector::CaptureAreaSelector(Capturer &capturer,
                                         const Settings &settings,
                                         const QPixmap &pixmap)
  : capturer_(capturer)
  , settings_(settings)
  , pixmap_(pixmap)
{
  setWindowFlags(Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint |
                 Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
  setCursor(Qt::CrossCursor);
  setMouseTracking(true);
  setAttribute(Qt::WA_OpaquePaintEvent);
}

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
Translation language: %2 (%3))")
              .arg(sourceName, targetName, translationState);
}

void CaptureAreaSelector::showEvent(QShowEvent * /*event*/)
{
  startSelectPos_ = currentSelectPos_ = QPoint();
}

void CaptureAreaSelector::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Escape)
    capturer_.canceled();
}

void CaptureAreaSelector::paintEvent(QPaintEvent * /*event*/)
{
  QPainter painter(this);
  painter.drawPixmap(rect(), pixmap_);

  for (const auto &rect : helpRects_) drawHelpRects(painter, rect);

  auto selection = QRect(startSelectPos_, currentSelectPos_).normalized();
  if (!selection.isValid())
    return;

  painter.setBrush({});
  painter.setPen(Qt::red);
  painter.drawRect(selection);
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

void CaptureAreaSelector::mousePressEvent(QMouseEvent *event)
{
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
  if (startSelectPos_.isNull() || pixmap_.isNull()) {
    capturer_.canceled();
    return;
  }

  const auto endPos = event->pos();
  const auto selection = QRect(startSelectPos_, endPos).normalized();

  startSelectPos_ = currentSelectPos_ = {};

  const auto area = CaptureArea(selection, settings_);
  capturer_.selected(area);
}

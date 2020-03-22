#include "captureareaselector.h"
#include "capturer.h"
#include "languagecodes.h"
#include "settings.h"
#include "task.h"

#include <QMouseEvent>
#include <QPainter>

CaptureAreaSelector::CaptureAreaSelector(Capturer &capturer,
                                         const Settings &settings)
  : capturer_(capturer)
  , settings_(settings)
{
  setWindowFlags(Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint |
                 Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
  setCursor(Qt::CrossCursor);
  setMouseTracking(true);
}

void CaptureAreaSelector::activate()
{
  show();
  activateWindow();
}

void CaptureAreaSelector::setPixmap(const QPixmap &pixmap)
{
  pixmap_ = pixmap;
  auto palette = this->palette();
  palette.setBrush(backgroundRole(), pixmap);
  setPalette(palette);
  setGeometry(pixmap_.rect());
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
  LanguageCodes languages;
  const auto source = languages.findById(settings_.sourceLanguage);
  const auto sourceName =
      source ? QObject::tr(source->name) : settings_.sourceLanguage;
  const auto target = languages.findById(settings_.targetLanguage);
  const auto targetName =
      target ? QObject::tr(target->name) : settings_.targetLanguage;

  help_ = tr(R"(Recognition language: %1
Translation language: %2)")
              .arg(sourceName, targetName);
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

  for (auto &screenHelp : helpRects_) {
    painter.setBrush(QBrush(QColor(200, 200, 200, 200)));
    painter.setPen(Qt::NoPen);
    painter.drawRect(screenHelp.current);

    painter.setBrush({});
    painter.setPen(Qt::black);
    painter.drawText(screenHelp.current, Qt::AlignCenter, help_);
  }

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
  const auto selectedPixmap = pixmap_.copy(selection);

  startSelectPos_ = currentSelectPos_ = QPoint();

  if (selectedPixmap.width() < 3 || selectedPixmap.height() < 3) {
    capturer_.canceled();
    return;
  }

  auto task = std::make_shared<Task>();
  task->captured = selectedPixmap;
  task->capturePoint = pos() + selection.topLeft();
  // TODO add customization menus
  capturer_.captured(task);
}

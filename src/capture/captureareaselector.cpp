#include "captureareaselector.h"
#include "capturer.h"
#include "languagecodes.h"
#include "settings.h"
#include "task.h"

#include <QMouseEvent>
#include <QPainter>
#include <QScreen>

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

void CaptureAreaSelector::setScreen(QScreen &screen)
{
  const auto geometry = screen.availableGeometry();
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
  const auto pixmap =
      screen.grabWindow(0, 0, 0, geometry.width(), geometry.height());
#else
  const auto pixmap = screen.grabWindow(0, geometry.x(), geometry.y(),
                                        geometry.width(), geometry.height());
#endif
  pixmap_ = pixmap;

  auto palette = this->palette();
  palette.setBrush(backgroundRole(), pixmap);
  setPalette(palette);
  setGeometry(geometry);

  updateHelp();
}

void CaptureAreaSelector::updateHelp()
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

  const auto rect = this->rect();
  auto helpRect = fontMetrics().boundingRect({}, 0, help_);
  helpRect.setSize(helpRect.size() * 1.4);
  helpRects_ = std::vector<QRect>(2, helpRect);
  helpRects_[0].moveTopLeft(rect.topLeft());
  helpRects_[1].moveTopRight(rect.topRight());
  currentHelpRect_ = helpRects_[0];
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

  const auto cursor = mapFromGlobal(QCursor::pos());
  if (currentHelpRect_.contains(cursor)) {
    for (const auto &rect : helpRects_) {
      if (!rect.contains(cursor)) {
        currentHelpRect_ = rect;
        break;
      }
    }
  }

  painter.setBrush(QBrush(QColor(200, 200, 200, 200)));
  painter.setPen(Qt::NoPen);
  painter.drawRect(currentHelpRect_);
  painter.setBrush({});
  painter.setPen(Qt::black);
  painter.drawText(currentHelpRect_, Qt::AlignCenter, help_);

  auto selection = QRect(startSelectPos_, currentSelectPos_).normalized();
  if (!selection.isValid())
    return;

  painter.setBrush({});
  painter.setPen(Qt::red);
  painter.drawRect(selection);
}

void CaptureAreaSelector::mousePressEvent(QMouseEvent *event)
{
  if (startSelectPos_.isNull())
    startSelectPos_ = event->pos();
}

void CaptureAreaSelector::mouseMoveEvent(QMouseEvent *event)
{
  if (startSelectPos_.isNull()) {
    if (currentHelpRect_.contains(event->pos()))
      update();
    return;
  }

  currentSelectPos_ = event->pos();
  update();
}

void CaptureAreaSelector::mouseReleaseEvent(QMouseEvent *event)
{
  if (startSelectPos_.isNull() || pixmap_.isNull())
    return;

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

#include "captureoverlay.h"
#include "capturer.h"
#include "task.h"

#include <QMouseEvent>
#include <QPainter>
#include <QScreen>

CaptureOverlay::CaptureOverlay(Capturer &capturer)
  : capturer_(capturer)
{
  setWindowFlags(Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint |
                 Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
  setCursor(Qt::CrossCursor);
}

void CaptureOverlay::setScreen(QScreen &screen)
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
}

void CaptureOverlay::showEvent(QShowEvent * /*event*/)
{
  startSelectPos_ = currentSelectPos_ = QPoint();
}

void CaptureOverlay::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Escape)
    capturer_.canceled();
}

void CaptureOverlay::paintEvent(QPaintEvent * /*event*/)
{
  auto selection = QRect(startSelectPos_, currentSelectPos_).normalized();
  if (!selection.isValid())
    return;

  QPainter painter(this);
  painter.setPen(Qt::red);
  painter.drawRect(selection);
}

void CaptureOverlay::mousePressEvent(QMouseEvent *event)
{
  if (startSelectPos_.isNull())
    startSelectPos_ = event->pos();
}

void CaptureOverlay::mouseMoveEvent(QMouseEvent *event)
{
  if (startSelectPos_.isNull())
    return;

  currentSelectPos_ = event->pos();
  repaint();
}

void CaptureOverlay::mouseReleaseEvent(QMouseEvent *event)
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

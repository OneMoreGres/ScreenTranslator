#include "resultwidget.h"
#include "debug.h"
#include "manager.h"
#include "representer.h"
#include "settings.h"
#include "task.h"

#include <QApplication>
#include <QBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QScreen>

ResultWidget::ResultWidget(Manager &manager, Representer &representer,
                           const Settings &settings, QWidget *parent)
  : QFrame(parent)
  , representer_(representer)
  , settings_(settings)
  , imagePlaceholder_(new QWidget(this))
  , image_(new QLabel(imagePlaceholder_))
  , recognized_(new QLabel(this))
  , separator_(new QLabel(this))
  , translated_(new QLabel(this))
  , contextMenu_(new QMenu(this))
{
  setWindowFlags(Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint |
                 Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);

  setLineWidth(1);
  setFrameShape(QFrame::StyledPanel);
  setFrameShadow(QFrame::Plain);

  recognized_->setAlignment(Qt::AlignCenter);
  recognized_->setWordWrap(true);

  separator_->setFixedHeight(1);
  separator_->setAutoFillBackground(true);

  translated_->setAlignment(Qt::AlignCenter);
  translated_->setWordWrap(true);

  {
    auto clipboardText = contextMenu_->addAction(tr("Copy text"));
    connect(clipboardText, &QAction::triggered,  //
            this, &ResultWidget::copyText);
    auto clipboardImage = contextMenu_->addAction(tr("Copy image"));
    connect(clipboardImage, &QAction::triggered,  //
            this, &ResultWidget::copyImage);
    auto edit = contextMenu_->addAction(tr("Edit..."));
    connect(edit, &QAction::triggered,  //
            this, &ResultWidget::edit);

    contextMenu_->addSeparator();

    auto capture = contextMenu_->addAction(tr("New capture"));
    connect(capture, &QAction::triggered,  //
            this, [&manager] { manager.capture(); });
    auto repeatCapture = contextMenu_->addAction(tr("Repeat capture"));
    connect(repeatCapture, &QAction::triggered,  //
            this, [&manager] { manager.repeatCapture(); });
  }

  installEventFilter(this);

  auto layout = new QVBoxLayout(this);
  layout->addWidget(imagePlaceholder_);
  layout->addWidget(recognized_);
  layout->addWidget(separator_);
  layout->addWidget(translated_);

  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  updateSettings();
}

const TaskPtr &ResultWidget::task() const
{
  return task_;
}

void ResultWidget::show(const TaskPtr &task)
{
  task_ = task;

  image_->setPixmap(task->captured);
  image_->resize(task->captured.size());
  imagePlaceholder_->setMinimumSize(image_->size());

  recognized_->setText(task->corrected);
  const auto tooltip = task->recognized == task->corrected
                           ? ""
                           : tr("Without correction:\n") + task->recognized;
  recognized_->setToolTip(tooltip);

  translated_->setText(task->translated);
  translated_->setToolTip(task->usedTranslator);

  const auto gotTranslation = !task->translated.isEmpty();
  translated_->setVisible(gotTranslation);
  separator_->setVisible(gotTranslation);

  const auto mustShowRecognized = settings_.showRecognized || !gotTranslation;
  recognized_->setVisible(mustShowRecognized);

  show();
  adjustSize();

  // window should not be smaller than selected image
  if (!imagePlaceholder_->isVisible())
    resize(std::max(width(), task->captured.width()),
           std::max(height(), task->captured.height()));

  // if window is wider than image then image should be at horizontal center
  const auto correctionToCenterImage =
      QPoint((width() - task->captured.width()) / 2, 2 * lineWidth());
  auto rect = QRect(task->capturePoint - correctionToCenterImage, size());

  auto screen = QApplication::screenAt(task->capturePoint);
  SOFT_ASSERT(screen, return );
  const auto screenRect = screen->geometry();

  // window should not exceed horizontal borders
  if (rect.right() > screenRect.right())
    rect.moveRight(screenRect.right());
  if (rect.left() < screenRect.left())
    rect.moveLeft(screenRect.left());

  // image should be where it was selected
  if (imagePlaceholder_->isVisible()) {
    const auto imageOffset =
        task->capturePoint.x() - rect.left() - 2 * lineWidth();
    image_->move(imageOffset, image_->y());
  }

  // window should not exceed vertical borders
  const auto shouldTextOnTop = rect.bottom() > screenRect.bottom();
  if (shouldTextOnTop)
    rect.moveBottom(rect.top() + task->captured.height() + 3 * lineWidth());

  auto layout = static_cast<QBoxLayout *>(this->layout());
  SOFT_ASSERT(layout, return );
  const auto isTextOnTop = layout->indexOf(recognized_) == 0;
  if (isTextOnTop != shouldTextOnTop) {
    layout->removeWidget(recognized_);
    layout->removeWidget(separator_);
    layout->removeWidget(translated_);
    auto index = shouldTextOnTop ? 0 : 1;
    layout->insertWidget(index, recognized_);
    layout->insertWidget(++index, separator_);
    layout->insertWidget(++index, translated_);
  }

  move(rect.topLeft());

  activateWindow();
}

void ResultWidget::updateSettings()
{
  QFont font(settings_.fontFamily, settings_.fontSize);
  setFont(font);

  auto palette = this->palette();
  const auto &backgroundColor = settings_.backgroundColor;
  palette.setColor(QPalette::Window, backgroundColor);
  palette.setColor(QPalette::WindowText, settings_.fontColor);
  setPalette(palette);

  const auto separatorColor = backgroundColor.lightness() > 150
                                  ? backgroundColor.darker()
                                  : backgroundColor.lighter();
  palette.setColor(QPalette::Window, separatorColor);
  separator_->setPalette(palette);

  imagePlaceholder_->setVisible(settings_.showCaptured);
}

void ResultWidget::mousePressEvent(QMouseEvent *event)
{
  const auto button = event->button();
  if (button == Qt::RightButton) {
    contextMenu_->exec(QCursor::pos());
    return;
  }

  if (button == Qt::MiddleButton) {
    lastPos_ = event->pos();
    return;
  }
}

void ResultWidget::mouseMoveEvent(QMouseEvent *event)
{
  if (!(event->buttons() & Qt::MiddleButton))
    return;

  move(pos() + event->pos() - lastPos_);
}

void ResultWidget::edit()
{
  representer_.edit(task_);
}

void ResultWidget::copyText()
{
  representer_.clipboardText(task_);
}

void ResultWidget::copyImage()
{
  representer_.clipboardImage(task_);
}

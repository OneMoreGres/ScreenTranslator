#include "resultwidget.h"
#include "debug.h"
#include "representer.h"
#include "settings.h"
#include "task.h"

#include <QApplication>
#include <QBoxLayout>
#include <QDesktopWidget>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>

ResultWidget::ResultWidget(Representer &representer, const Settings &settings,
                           QWidget *parent)
  : QFrame(parent)
  , representer_(representer)
  , settings_(settings)
  , image_(new QLabel(this))
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

  image_->setAlignment(Qt::AlignCenter);

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
  }

  installEventFilter(this);

  auto layout = new QVBoxLayout(this);
  layout->addWidget(image_);
  layout->addWidget(recognized_);
  layout->addWidget(separator_);
  layout->addWidget(translated_);

  layout->setMargin(0);
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

  recognized_->setText(task->corrected);
  const auto tooltip = task->recognized == task->corrected
                           ? ""
                           : tr("Without correction:\n") + task->recognized;
  recognized_->setToolTip(tooltip);

  translated_->setText(task->translated);

  const auto gotTranslation = !task->translated.isEmpty();
  translated_->setVisible(gotTranslation);
  separator_->setVisible(gotTranslation);

  const auto mustShowRecognized = settings_.showRecognized || !gotTranslation;
  recognized_->setVisible(mustShowRecognized);

  show();
  adjustSize();

  if (!image_->isVisible())
    resize(std::max(width(), task->captured.width()),
           std::max(height(), task->captured.height()));

  QDesktopWidget *desktop = QApplication::desktop();
  Q_CHECK_PTR(desktop);
  const auto correction =
      QPoint((width() - task->captured.width()) / 2, lineWidth());
  auto rect = QRect(task->capturePoint - correction, size());

  const auto screenRect = desktop->screenGeometry(this);
  const auto shouldTextOnTop = rect.bottom() > screenRect.bottom();
  if (shouldTextOnTop)
    rect.moveBottom(rect.top() + task->captured.height() + lineWidth());

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

  image_->setVisible(settings_.showCaptured);
}

bool ResultWidget::eventFilter(QObject *watched, QEvent *event)
{
  if (event->type() == QEvent::WindowDeactivate)
    hide();
  return QWidget::eventFilter(watched, event);
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

  hide();
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

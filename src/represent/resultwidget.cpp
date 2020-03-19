#include "resultwidget.h"
#include "debug.h"
#include "task.h"

#include <QApplication>
#include <QBoxLayout>
#include <QDesktopWidget>
#include <QLabel>
#include <QMouseEvent>

ResultWidget::ResultWidget(QWidget *parent)
  : QFrame(parent)
  , image_(new QLabel(this))
  , recognized_(new QLabel(this))
  , translated_(new QLabel(this))
{
  setWindowFlags(Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint |
                 Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);

  setLineWidth(1);
  setFrameShape(QFrame::StyledPanel);
  setFrameShadow(QFrame::Plain);

  image_->setAlignment(Qt::AlignCenter);

  recognized_->setObjectName("recognizeLabel");
  recognized_->setAlignment(Qt::AlignCenter);
  recognized_->setWordWrap(true);

  translated_->setObjectName("translateLabel");
  translated_->setAlignment(Qt::AlignCenter);
  translated_->setWordWrap(true);

  const auto styleSheet =
      "#recognizeLabel, #translateLabel {"
      "color: black;"
      "background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
      "stop:0 darkGray, stop: 0.5 lightGray, stop:1 darkGray);"
      "}";
  setStyleSheet(styleSheet);

  installEventFilter(this);

  auto layout = new QVBoxLayout(this);
  layout->addWidget(image_);
  layout->addWidget(recognized_);
  layout->addWidget(translated_);

  layout->setMargin(0);
  layout->setSpacing(1);
}

void ResultWidget::show(const TaskPtr &task)
{
  SOFT_ASSERT(task->isValid(), return );
  image_->setPixmap(task->captured);

  recognized_->setText(task->corrected);
  const auto tooltip = task->recognized == task->corrected
                           ? ""
                           : tr("Without correction:\n") + task->recognized;
  recognized_->setToolTip(tooltip);

  translated_->setText(task->translated);

  const auto gotTranslation = !task->translated.isEmpty();
  translated_->setVisible(gotTranslation);

  const auto mustShowRecognized = showRecognized_ || !gotTranslation;
  recognized_->setVisible(mustShowRecognized);

  show();
  adjustSize();

  QDesktopWidget *desktop = QApplication::desktop();
  Q_CHECK_PTR(desktop);
  auto correction = QPoint((width() - task->captured.width()) / 2, lineWidth());
  move(task->capturePoint - correction);

  auto screenRect = desktop->screenGeometry(this);
  auto minY = screenRect.bottom() - height();
  if (y() > minY) {
    move(x(), minY);
  }

  activateWindow();
}

void ResultWidget::updateSettings(const QFont &font, bool showRecognized,
                                  bool showCaptured)
{
  // explicit font change because of stylesheet
  recognized_->setFont(font);
  translated_->setFont(font);

  image_->setVisible(showCaptured);
  showRecognized_ = showRecognized;
}

bool ResultWidget::eventFilter(QObject *watched, QEvent *event)
{
  if (event->type() == QEvent::MouseButtonPress) {
    const auto button = static_cast<QMouseEvent *>(event)->button();
    if (button == Qt::LeftButton) {
      hide();
    }
    //    else if (button == Qt::RightButton) {
    //      QAction *action = contextMenu_->exec(QCursor::pos());
    //      if (recognizeSubMenu_->findChildren<QAction *>().contains(action)) {
    //        ProcessingItem item = item_;
    //        task->translated = task->recognized = QString();
    //        task->ocrLanguage = dictionary_.ocrUiToCode(action->text());
    //        emit requestRecognize(item);
    //      } else if (translateSubMenu_->findChildren<QAction *>().contains(
    //                     action)) {
    //        ProcessingItem item = item_;
    //        task->translated.clear();
    //        task->translateLanguage =
    //        dictionary_.translateUiToCode(action->text()); emit
    //        requestTranslate(item);
    //      } else if (action == clipboardAction_) {
    //        emit requestClipboard();
    //      } else if (action == imageClipboardAction_) {
    //        emit requestImageClipboard();
    //      } else if (action == correctAction_) {
    //        emit requestEdition(item_);
    //        // Return because Manager calls showResult() before hide()
    //        otherwise.return QWidget::eventFilter(watched, event);
    //      }
    //    }
    //    hide();
  } else if (event->type() == QEvent::WindowDeactivate) {
    hide();
  }
  return QWidget::eventFilter(watched, event);
}

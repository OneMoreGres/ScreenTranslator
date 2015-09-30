#include "ResultDialog.h"
#include "ui_ResultDialog.h"
#include "StAssert.h"
#include "LanguageHelper.h"

#include <QDesktopWidget>
#include <QMouseEvent>
#include <QMenu>

ResultDialog::ResultDialog (const LanguageHelper &dictionary, QWidget *parent) :
  QDialog (parent),
  ui (new Ui::ResultDialog),
  dictionary_ (dictionary), isShowAtCapturePos_ (true),
  contextMenu_ (NULL), recognizeSubMenu_ (NULL), clipboardAction_ (NULL) {
  ui->setupUi (this);
  setWindowFlags (Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint |
                  Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);

  installEventFilter (this);
  createContextMenu ();
  applySettings ();
}

ResultDialog::~ResultDialog () {
  delete contextMenu_;
  delete ui;
}

const ProcessingItem &ResultDialog::item () const {
  return item_;
}

void ResultDialog::applySettings () {
  dictionary_.updateMenu (recognizeSubMenu_, dictionary_.availableOcrLanguagesUi ());
}

void ResultDialog::createContextMenu () {
  contextMenu_ = new QMenu ();
  recognizeSubMenu_ = contextMenu_->addMenu (tr ("Распознать другой язык"));
  clipboardAction_ = contextMenu_->addAction (tr ("Скопировать в буфер"));
}

bool ResultDialog::eventFilter (QObject *object, QEvent *event) {
  Q_UNUSED (object);
  if (event->type () == QEvent::MouseButtonPress) {
    Qt::MouseButton button = static_cast<QMouseEvent *>(event)->button ();
    if (button == Qt::RightButton) {
      QAction *action = contextMenu_->exec (QCursor::pos ());
      if (recognizeSubMenu_->findChildren<QAction *> ().contains (action)) {
        ProcessingItem item = item_;
        item.translated = item.recognized = QString ();
        item.ocrLanguage = dictionary_.ocrUiToCode (action->text ());
        emit requestRecognize (item);
      }
      else if (action == clipboardAction_) {
        emit requestClipboard ();
      }
    }
    hide ();
  }
  else if (event->type () == QEvent::WindowDeactivate) {
    hide ();
  }
  return QDialog::eventFilter (object, event);
}

void ResultDialog::showResult (ProcessingItem item) {
  ST_ASSERT (item.isValid ());
  item_ = item;
  ui->sourceLabel->setPixmap (item.source);
  ui->recognizeLabel->setText (item.recognized);
  ui->translateLabel->setText (item.translated);
  bool gotTranslation = !item.translated.isEmpty ();
  ui->translateLabel->setVisible (gotTranslation);
  ui->translateLine->setVisible (gotTranslation);

  show ();
  adjustSize ();

  QDesktopWidget *desktop = QApplication::desktop ();
  Q_CHECK_PTR (desktop);
  if (isShowAtCapturePos_) {
    QPoint correction = QPoint (ui->frame->lineWidth (), ui->frame->lineWidth ());
    move (item.screenPos - correction);
    QRect screenRect = desktop->screenGeometry (this);
    int minY = screenRect.bottom () - height ();
    if (y () > minY) {
      move (x (), minY);
    }
  }
  else {

    QRect screenRect = desktop->availableGeometry (this);
    ST_ASSERT (screenRect.isValid ());
    QPoint newPos (screenRect.width () - width (), screenRect.height () - height ());
    move (newPos);
  }
  activateWindow ();
}

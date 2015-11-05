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
  dictionary_ (dictionary),
  contextMenu_ (NULL), recognizeSubMenu_ (NULL),  translateSubMenu_ (NULL),
  clipboardAction_ (NULL), imageClipboardAction_ (NULL), correctAction_ (NULL) {
  ui->setupUi (this);
  setWindowFlags (Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint |
                  Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);

  QString styleSheet =
    "#recognizeLabel, #translateLabel {"
    "color: black;"
    "background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
    "stop:0 darkGray, stop: 0.5 lightGray, stop:1 darkGray);"
    "}";
  setStyleSheet (styleSheet);

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
  dictionary_.updateMenu (translateSubMenu_, dictionary_.translateLanguagesUi ());
}

void ResultDialog::createContextMenu () {
  contextMenu_ = new QMenu ();
  recognizeSubMenu_ = contextMenu_->addMenu (tr ("Распознать другой язык"));
  translateSubMenu_ = contextMenu_->addMenu (tr ("Перевести на другой язык"));
  clipboardAction_ = contextMenu_->addAction (tr ("Скопировать в буфер"));
  imageClipboardAction_ = contextMenu_->addAction (tr ("Скопировать рисунок в буфер"));
  correctAction_ = contextMenu_->addAction (tr ("Исправить распознанный текст"));
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
      else if (translateSubMenu_->findChildren<QAction *> ().contains (action)) {
        ProcessingItem item = item_;
        item.translated.clear ();
        item.translateLanguage = dictionary_.translateUiToCode (action->text ());
        emit requestTranslate (item);
      }
      else if (action == clipboardAction_) {
        emit requestClipboard ();
      }
      else if (action == imageClipboardAction_) {
        emit requestImageClipboard ();
      }
      else if (action == correctAction_) {
        emit requestEdition (item_);
        // Return because Manager calls showResult() before hide() otherwise.
        return QDialog::eventFilter (object, event);
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
#ifdef Q_OS_LINUX
  hide (); // buggy otherwise (on some systems)
  show ();
#endif

  QDesktopWidget *desktop = QApplication::desktop ();
  Q_CHECK_PTR (desktop);
  QPoint correction = QPoint ((width () - item.source.width ()) / 2, ui->frame->lineWidth ());
  move (item.screenPos - correction);
  QRect screenRect = desktop->screenGeometry (this);
  int minY = screenRect.bottom () - height ();
  if (y () > minY) {
    move (x (), minY);
  }
  activateWindow ();
}

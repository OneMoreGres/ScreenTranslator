#include "SelectionDialog.h"
#include "ui_SelectionDialog.h"
#include "LanguageHelper.h"
#include "StAssert.h"

#include <QMouseEvent>
#include <QPainter>
#include <QDebug>
#include <QMenu>

SelectionDialog::SelectionDialog (const LanguageHelper &dictionary, QWidget *parent) :
  QDialog (parent),
  ui (new Ui::SelectionDialog), dictionary_ (dictionary),
  languageMenu_ (new QMenu), swapLanguagesAction_ (NULL) {
  ui->setupUi (this);
  setWindowFlags (Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint |
                  Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);

  ui->label->setAutoFillBackground (false);
  ui->label->installEventFilter (this);

  applySettings ();
}

SelectionDialog::~SelectionDialog () {
  delete languageMenu_;
  delete ui;
}

void SelectionDialog::applySettings () {
  dictionary_.updateMenu (languageMenu_, dictionary_.availableOcrLanguagesUi ());
  if (!languageMenu_->isEmpty ()) {
    swapLanguagesAction_ = languageMenu_->addAction (tr ("Поменять язык текста и перевода"));
  }
}

bool SelectionDialog::eventFilter (QObject *object, QEvent *event) {
  if (object != ui->label) {
    return QDialog::eventFilter (object, event);
  }

  if (event->type () == QEvent::Show) {
    startSelectPos_ = currentSelectPos_ = QPoint ();
  }
  else if (event->type () == QEvent::MouseButtonPress) {
    QMouseEvent *mouseEvent = static_cast <QMouseEvent *> (event);
    if ((mouseEvent->button () == Qt::LeftButton ||
         mouseEvent->button () == Qt::RightButton) && startSelectPos_.isNull ()) {
      startSelectPos_ = mouseEvent->pos ();
    }
  }
  else if (event->type () == QEvent::MouseMove) {
    QMouseEvent *mouseEvent = static_cast <QMouseEvent *> (event);
    if ((mouseEvent->buttons () & Qt::LeftButton ||
         mouseEvent->buttons () & Qt::RightButton) && !startSelectPos_.isNull ()) {
      currentSelectPos_ = mouseEvent->pos ();
      ui->label->repaint ();
    }
  }
  else if (event->type () == QEvent::Paint) {
    QRect selection = QRect (startSelectPos_, currentSelectPos_).normalized ();
    if (selection.isValid ()) {
      QPainter painter (ui->label);
      painter.setPen (Qt::red);
      painter.drawRect (selection);
    }
  }
  else if (event->type () == QEvent::MouseButtonRelease) {
    QMouseEvent *mouseEvent = static_cast <QMouseEvent *> (event);
    if (mouseEvent->button () == Qt::LeftButton ||
        mouseEvent->button () == Qt::RightButton) {
      if (startSelectPos_.isNull () || currentPixmap_.isNull ()) {
        return QDialog::eventFilter (object, event);
      }
      QPoint endPos = mouseEvent->pos ();
      QRect selection = QRect (startSelectPos_, endPos).normalized ();
      startSelectPos_ = currentSelectPos_ = QPoint ();
      QPixmap selectedPixmap = currentPixmap_.copy (selection);
      if (selectedPixmap.width () < 3 || selectedPixmap.height () < 3) {
        reject ();
        return QDialog::eventFilter (object, event);
      }
      ProcessingItem item;
      item.source = selectedPixmap;
      item.screenPos = pos () + selection.topLeft ();
      item.modifiers = mouseEvent->modifiers ();

      if (mouseEvent->button () == Qt::RightButton &&
          !languageMenu_->children ().isEmpty ()) {
        QAction *action = languageMenu_->exec (QCursor::pos ());
        if (action == NULL) {
          reject ();
          return QDialog::eventFilter (object, event);
        }
        if (action == swapLanguagesAction_) {
          item.swapLanguages_ = true;
        }
        else {
          item.ocrLanguage = dictionary_.ocrUiToCode (action->text ());
          ST_ASSERT (!item.ocrLanguage.isEmpty ());
          item.sourceLanguage = dictionary_.ocrToTranslateCodes (item.ocrLanguage);
          ST_ASSERT (!item.sourceLanguage.isEmpty ());
        }
      }
      emit selected (item);
    }
  }
  return QDialog::eventFilter (object, event);
}

void SelectionDialog::setPixmap (QPixmap pixmap, const QRect &showGeometry) {
  ST_ASSERT (!pixmap.isNull ());
  ST_ASSERT (!showGeometry.isEmpty ());
  currentPixmap_ = pixmap;
  QPalette palette = this->palette ();
  palette.setBrush (this->backgroundRole (), pixmap);
  this->setPalette (palette);
  this->setGeometry (showGeometry);

  show ();
  activateWindow ();
}

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
  languageMenu_ (new QMenu) {
  ui->setupUi (this);
  setWindowFlags (Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint |
                  Qt::WindowStaysOnTopHint);

  ui->label->setAutoFillBackground (false);
  ui->label->installEventFilter (this);

  updateMenu ();
}

SelectionDialog::~SelectionDialog () {
  delete ui;
}

void SelectionDialog::updateMenu () {
  Q_CHECK_PTR (languageMenu_);
  languageMenu_->clear ();
  QStringList languages = dictionary_.availableOcrLanguagesUi ();
  if (languages.isEmpty ()) {
    return;
  }

  const int max = 10;

  if (languages.size () <= max) {
    foreach (const QString &language, languages) {
      languageMenu_->addAction (language);
    }
  }
  else {
    int subIndex = max;
    QMenu *subMenu = NULL;
    QString prevLetter;
    foreach (const QString &language, languages) {
      QString curLetter = language.left (1);
      if (++subIndex >= max && prevLetter != curLetter) {
        if (subMenu != NULL) {
          subMenu->setTitle (subMenu->title () + " - " + prevLetter);
        }
        subMenu = languageMenu_->addMenu (curLetter);
        subIndex = 0;
      }
      prevLetter = curLetter;
      subMenu->addAction (language);
    }
    subMenu->setTitle (subMenu->title () + " - " + prevLetter);
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
      QPixmap selectedPixmap = currentPixmap_.copy (selection);
      if (!selectedPixmap.isNull ()) {
        ProcessingItem item;
        item.source = selectedPixmap;
        item.screenPos = selection.topLeft ();

        if (mouseEvent->button () == Qt::RightButton &&
            !languageMenu_->children ().isEmpty ()) {
          QAction *action = languageMenu_->exec (QCursor::pos ());
          if (action == NULL) {
            reject ();
            return QDialog::eventFilter (object, event);
          }
          item.ocrLanguage = dictionary_.ocrUiToCode (action->text ());
          ST_ASSERT (!item.ocrLanguage.isEmpty ());
          item.sourceLanguage = dictionary_.translateForOcrCode (item.ocrLanguage);
          ST_ASSERT (!item.sourceLanguage.isEmpty ());
        }
        emit selected (item);
      }
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
  setFocus ();
}

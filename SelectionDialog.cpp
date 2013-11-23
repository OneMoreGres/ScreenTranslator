#include "SelectionDialog.h"
#include "ui_SelectionDialog.h"

#include <QMouseEvent>
#include <QPainter>
#include <QDebug>

SelectionDialog::SelectionDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SelectionDialog)
{
  ui->setupUi(this);
  setWindowFlags (Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint |
                  Qt::WindowStaysOnTopHint);

  ui->label->setAutoFillBackground(false);
  ui->label->installEventFilter (this);
}

SelectionDialog::~SelectionDialog()
{
  delete ui;
}

bool SelectionDialog::eventFilter(QObject* object, QEvent* event)
{
  if (object != ui->label)
  {
    return QDialog::eventFilter (object, event);
  }

  if (event->type () == QEvent::MouseButtonPress)
  {
    QMouseEvent* mouseEvent = static_cast <QMouseEvent*> (event);
    if (mouseEvent->button () == Qt::LeftButton)
    {
      startSelectPos_ = mouseEvent->pos ();
    }
  }
  else if (event->type () == QEvent::MouseButtonRelease)
  {
    QMouseEvent* mouseEvent = static_cast <QMouseEvent*> (event);
    if (mouseEvent->button () == Qt::LeftButton)
    {
      if (startSelectPos_.isNull () || currentPixmap_.isNull ())
      {
        QDialog::eventFilter (object, event);
      }
      QPoint endPos = mouseEvent->pos ();
      QRect selection = QRect (startSelectPos_, endPos).normalized ();
      startSelectPos_ = currentSelectPos_ = QPoint ();
      QPixmap selectedPixmap = currentPixmap_.copy (selection);
      emit selected (selectedPixmap);
//      accept (); //DEBUG
    }
  }
  else if (event->type () == QEvent::MouseMove)
  {
    QMouseEvent* mouseEvent = static_cast <QMouseEvent*> (event);
    if (mouseEvent->buttons () & Qt::LeftButton)
    {
      currentSelectPos_ = mouseEvent->pos ();
      ui->label->repaint ();
    }
  }
  else if (event->type () == QEvent::Paint)
  {
    QPainter painter (ui->label);
    painter.setPen (Qt::red);
    QRect selection = QRect (startSelectPos_, currentSelectPos_).normalized ();
    painter.drawRect (selection);
  }

  return QDialog::eventFilter (object, event);
}

void SelectionDialog::setPixmap(QPixmap pixmap)
{
  Q_ASSERT (!pixmap.isNull ());
  currentPixmap_ = pixmap;
  QPalette palette = this->palette ();
  palette.setBrush (this->backgroundRole (), pixmap);
  this->setPalette (palette);
  this->resize (pixmap.size ());

  show ();
  setFocus ();
}

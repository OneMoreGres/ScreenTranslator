#include "ResultDialog.h"
#include "ui_ResultDialog.h"

#include <QDesktopWidget>

ResultDialog::ResultDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ResultDialog),
  isShowAtCapturePos_ (true)
{
  ui->setupUi(this);
  setWindowFlags (Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint |
                  Qt::WindowStaysOnTopHint);

  installEventFilter (this);
}

ResultDialog::~ResultDialog()
{
  delete ui;
}

bool ResultDialog::eventFilter(QObject* object, QEvent* event)
{
  Q_UNUSED (object);
  if (event->type () == QEvent::MouseButtonRelease)
  {
    hide ();
  }
  return QDialog::eventFilter (object, event);
}

void ResultDialog::showResult(ProcessingItem item)
{
  Q_ASSERT (!item.source.isNull ());
  Q_ASSERT (!item.recognized.isEmpty ());
  Q_ASSERT (!item.translated.isEmpty ());
  Q_ASSERT (!item.screenPos.isNull ());

  ui->sourceLabel->setPixmap (item.source);
  ui->recognizeLabel->setText (item.recognized);
  ui->translateLabel->setText (item.translated);

  show ();
  adjustSize ();

  QDesktopWidget* desktop = QApplication::desktop ();
  Q_CHECK_PTR (desktop);
  if (isShowAtCapturePos_)
  {
    QPoint correction = QPoint (ui->frame->lineWidth (), ui->frame->lineWidth ());
    move (item.screenPos - correction);
    QRect screenRect = desktop->screenGeometry (this);
    int minY = screenRect.bottom () - height ();
    if (y () > minY)
    {
      move (x (), minY);
    }
  }
  else
  {

    QRect screenRect = desktop->availableGeometry (this);
    Q_ASSERT (screenRect.isValid ());
    QPoint newPos (screenRect.width () - width (), screenRect.height () - height ());
    move (newPos);
  }
}

#ifndef RESULTDIALOG_H
#define RESULTDIALOG_H

#include <QDialog>

#include "ProcessingItem.h"

namespace Ui {
  class ResultDialog;
}

class ResultDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit ResultDialog(QWidget *parent = 0);
    ~ResultDialog();

  public:
    bool eventFilter (QObject *object, QEvent *event);

  public slots:
    void showResult (ProcessingItem item);

  private:
    Ui::ResultDialog *ui;
    bool isShowAtCapturePos_;
};

#endif // RESULTDIALOG_H

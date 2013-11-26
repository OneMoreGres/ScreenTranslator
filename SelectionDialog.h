#ifndef SELECTIONDIALOG_H
#define SELECTIONDIALOG_H

#include <QDialog>
#include <QPixmap>

#include "ProcessingItem.h"

namespace Ui {
  class SelectionDialog;
}

class SelectionDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit SelectionDialog(QWidget *parent = 0);
    ~SelectionDialog();

    bool eventFilter (QObject *object, QEvent *event);

  signals:
    void selected (ProcessingItem pixmap);

  public slots:
    void setPixmap (QPixmap pixmap);

  private:
    Ui::SelectionDialog *ui;
    QPoint startSelectPos_;
    QPoint currentSelectPos_;
    QPixmap currentPixmap_;
};

#endif // SELECTIONDIALOG_H

#ifndef SELECTIONDIALOG_H
#define SELECTIONDIALOG_H

#include <QDialog>
#include <QPixmap>
#include <QMenu>

#include "ProcessingItem.h"

namespace Ui {
  class SelectionDialog;
}
class LanguageHelper;

class SelectionDialog : public QDialog {
  Q_OBJECT

  public:
    explicit SelectionDialog (const LanguageHelper &dictionary, QWidget *parent = 0);
    ~SelectionDialog ();

    bool eventFilter (QObject *object, QEvent *event);

  signals:
    void selected (ProcessingItem pixmap);
    void nothingSelected ();

  public slots:
    //! Show pixmap with given geometry.
    void setPixmap (QPixmap pixmap, const QRect &showGeometry);
    void updateMenu ();

  private:
    Ui::SelectionDialog *ui;
    const LanguageHelper &dictionary_;
    QPoint startSelectPos_;
    QPoint currentSelectPos_;
    QPixmap currentPixmap_;
    QMenu *languageMenu_;
};

#endif // SELECTIONDIALOG_H

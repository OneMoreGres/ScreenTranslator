#ifndef RESULTDIALOG_H
#define RESULTDIALOG_H

#include <QDialog>
#include <QMenu>

#include "ProcessingItem.h"

namespace Ui {
  class ResultDialog;
}
class LanguageHelper;

class ResultDialog : public QDialog {
  Q_OBJECT

  public:
    explicit ResultDialog (const LanguageHelper &dictionary, QWidget *parent = 0);
    ~ResultDialog ();

  signals:
    void requestRecognize (ProcessingItem item);
    void requestTranslate (ProcessingItem item);
    void requestClipboard (); // Assume that slot will be called immediately.

  public:
    const ProcessingItem &item () const;
    bool eventFilter (QObject *object, QEvent *event);

  public slots:
    void showResult (ProcessingItem item);
    void applySettings ();

  private:
    void createContextMenu ();

  private:
    Ui::ResultDialog *ui;
    const LanguageHelper &dictionary_;
    bool isShowAtCapturePos_;
    QMenu *contextMenu_;
    QMenu *recognizeSubMenu_;
    QMenu *translateSubMenu_;
    QAction *clipboardAction_;
    ProcessingItem item_;
};

#endif // RESULTDIALOG_H

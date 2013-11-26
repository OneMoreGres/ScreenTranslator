#ifndef MANAGER_H
#define MANAGER_H

#include <QPixmap>
#include <QSystemTrayIcon>

#include "ProcessingItem.h"

class QAction;
class QMenu;

class SelectionDialog;
class ResultDialog;

class Manager : public QObject
{
    Q_OBJECT
  public:
    explicit Manager(QObject *parent = 0);
    ~Manager ();

  signals:
    void showPixmap (QPixmap pixmap);
    void settingsEdited ();

  private slots:
    void capture ();
    void settings ();
    void close ();
    void about ();
    void showLast ();
    void copyLastToClipboard ();

    void applySettings ();

    void processTrayAction (QSystemTrayIcon::ActivationReason reason);

    void showResult (ProcessingItem item);
    void showError (QString text);

  private:
    QMenu* trayContextMenu ();

  private:
    QSystemTrayIcon* trayIcon_;
    SelectionDialog* selection_;
    ResultDialog* resultDialog_;
    QAction* captureAction_;
    QAction* repeatAction_;
    QAction* clipboardAction_;
    ProcessingItem lastItem_;
    bool useResultDialog_;
};

#endif // MANAGER_H

#ifndef MANAGER_H
#define MANAGER_H

#include <QPixmap>
#include <QSystemTrayIcon>
#include <QMap>

#include "ProcessingItem.h"

class QAction;
class QMenu;

class SelectionDialog;
class ResultDialog;
class LanguageHelper;

class Manager : public QObject {
  Q_OBJECT

  public:
    explicit Manager (QObject *parent = 0);
    ~Manager ();

  signals:
    void requestRecognize (ProcessingItem item);
    void recognized (ProcessingItem item);
    void requestTranslate (ProcessingItem item);
    void closeSelections ();
    void settingsEdited ();

  private slots:
    void capture ();
    void repeatCapture ();
    void settings ();
    void close ();
    void about ();
    void showLast ();
    void copyLastToClipboard ();

    void applySettings ();

    void processTrayAction (QSystemTrayIcon::ActivationReason reason);

    void editRecognized (ProcessingItem item);
    void handleSelection (ProcessingItem item);
    void showResult (ProcessingItem item);
    void showError (QString text);

  private:
    QMenu * trayContextMenu ();
    void updateActionsState (bool isEnabled = true);

  private:
    QSystemTrayIcon *trayIcon_;
    LanguageHelper *dictionary_;
    //! Selection dialogs for each screen. Key - screen name.
    QMap<QString, SelectionDialog *> selections_;
    ResultDialog *resultDialog_;
    QAction *captureAction_;
    QAction *repeatCaptureAction_;
    QAction *repeatAction_;
    QAction *clipboardAction_;
    bool useResultDialog_;
    //! Used threads. For proper termination.
    QList<QThread *> threads_;
    QString defaultTranslationLanguage_;
};

#endif // MANAGER_H

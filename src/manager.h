#ifndef MANAGER_H
#define MANAGER_H

#include <QPixmap>
#include <QSystemTrayIcon>
#include <QMap>

#include "processingitem.h"

class QAction;
class QMenu;

class SelectionDialog;
class ResultDialog;
class LanguageHelper;
class Updater;

class Manager : public QObject {
  Q_OBJECT

  enum IconType {
    IconTypeNormal, IconTypeWorking, IconTypeError, IconTypeSuccess
  };

  public:
    explicit Manager (QObject *parent = 0);
    ~Manager ();

  signals:
    void requestRecognize (ProcessingItem item);
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
    void copyLastImageToClipboard ();

    void applySettings ();
    void checkForUpdates ();

    void processTrayAction (QSystemTrayIcon::ActivationReason reason);

    void editRecognized (ProcessingItem item);
    void handleSelection (ProcessingItem item);
    void showResult (ProcessingItem item);
    void showError (QString text);

    void updateNormalIcon ();

  private:
    QMenu * trayContextMenu ();
    void updateActionsState (bool isEnabled = true);
    void changeIcon (int iconType, int timeoutMsec = 3000);
    void scheduleUpdate (bool justChecked = false);

  private:
    QSystemTrayIcon *trayIcon_;
    LanguageHelper *dictionary_;
    //! Selection dialogs for each screen. Key - screen name.
    QMap<QString, SelectionDialog *> selections_;
    ResultDialog *resultDialog_;
    Updater *updater_;
    QTimer *updateTimer_;
    QAction *captureAction_;
    QAction *repeatCaptureAction_;
    QAction *repeatAction_;
    QAction *clipboardAction_;
    bool useResultDialog_;
    //! Used threads. For proper termination.
    QList<QThread *> threads_;
    QString defaultTranslationLanguage_;
    QString defaultOrcLanguage_;
    bool doTranslation_;
    int itemProcessingCount_;
};

#endif // MANAGER_H

#ifndef MANAGER_H
#define MANAGER_H

#include <QPixmap>
#include <QSystemTrayIcon>

class QAction;
class QMenu;

class SelectionDialog;

class Manager : public QObject
{
    Q_OBJECT
  public:
    explicit Manager(QObject *parent = 0);
    ~Manager ();

  signals:
    void showPixmap (QPixmap pixmap);
    void recognize (QPixmap pixmap);
    void settingsEdited ();

  private slots:
    void capture ();
    void settings ();
    void close ();
    void about ();

    void applySettings ();

    void processTrayAction (QSystemTrayIcon::ActivationReason reason);

    void showTranslation (QString sourceText, QString translatedText);
    void showError (QString text);

  private:
    QMenu* trayContextMenu ();

  private:
    QSystemTrayIcon* trayIcon_;
    SelectionDialog* selection_;
    QAction* captureAction_;
    QString lastMessage_;
};

#endif // MANAGER_H

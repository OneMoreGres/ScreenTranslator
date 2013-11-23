#ifndef MANAGER_H
#define MANAGER_H

#include <QObject>
#include <QPixmap>

class QMenu;
class QSystemTrayIcon;

class SelectionDialog;

class Manager : public QObject
{
    Q_OBJECT
  public:
    explicit Manager(QObject *parent = 0);
    ~Manager ();

  signals:
    void showPixmap (QPixmap pixmap);

  private slots:
    void capture ();
    void settings ();
    void close ();

    void processRegion (QPixmap selected);

  private:
    QMenu* trayContextMenu ();

  private:
    QSystemTrayIcon* trayIcon_;
    SelectionDialog* selection_;

};

#endif // MANAGER_H

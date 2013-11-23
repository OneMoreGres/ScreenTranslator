#include "Manager.h"

#include <QDebug>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>

#include "SettingsEditor.h"
#include "SelectionDialog.h"
#include "GlobalActionHelper.h"

Manager::Manager(QObject *parent) :
  QObject(parent),
  trayIcon_ (new QSystemTrayIcon (QIcon (":/images/icon.png"), this)),
  selection_ (new SelectionDialog)
{
  trayIcon_->show ();
  trayIcon_->setContextMenu (trayContextMenu ());

  selection_->setWindowIcon (trayIcon_->icon ());
  connect (this, SIGNAL (showPixmap (QPixmap)),
           selection_, SLOT (setPixmap (QPixmap)));
  connect (selection_, SIGNAL (selected (QPixmap)),
            SLOT (processRegion (QPixmap)));

  GlobalActionHelper::init ();
}

Manager::~Manager()
{
}

QMenu*Manager::trayContextMenu()
{
  QMenu* menu = new QMenu ();
  QAction* capture = menu->addAction (tr ("Захват"), this, SLOT (capture ()),
                                      tr ("Ctrl+Alt+Z"));
  GlobalActionHelper::makeGlobal (capture);
  menu->addAction (tr ("Настройки"), this, SLOT (settings ()));
  menu->addAction (tr ("Выход"), this, SLOT (close ()));
  return menu;
}

void Manager::capture()
{
  QList<QScreen*> screens = QApplication::screens ();
  Q_ASSERT (!screens.isEmpty ());
  QScreen* screen = screens.first ();
  Q_CHECK_PTR (screen);
  WId desktopId = QApplication::desktop ()->winId ();
  QPixmap pixmap = screen->grabWindow (desktopId);
  Q_ASSERT (!pixmap.isNull ());
  emit showPixmap (pixmap);
}

void Manager::settings()
{
  SettingsEditor editor;
  editor.setWindowIcon (trayIcon_->icon ());
  editor.exec ();
}

void Manager::close()
{
  QApplication::quit ();
}

void Manager::processRegion(QPixmap selected)
{

}

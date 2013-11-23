#include "Manager.h"

#include <QDebug>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QThread>

#include "SettingsEditor.h"
#include "SelectionDialog.h"
#include "GlobalActionHelper.h"
#include "Recognizer.h"
#include "Translator.h"

Manager::Manager(QObject *parent) :
  QObject(parent),
  trayIcon_ (new QSystemTrayIcon (QIcon (":/images/icon.png"), this)),
  selection_ (new SelectionDialog)
{
  GlobalActionHelper::init ();

  selection_->setWindowIcon (trayIcon_->icon ());
  connect (this, SIGNAL (showPixmap (QPixmap)),
           selection_, SLOT (setPixmap (QPixmap)));

  Recognizer* recognizer = new Recognizer;
  connect (selection_, SIGNAL (selected (QPixmap)),
           recognizer, SLOT (recognize (QPixmap)));
  QThread* recognizerThread = new QThread (this);
  recognizer->moveToThread (recognizerThread);
  recognizerThread->start ();

  Translator* translator = new Translator;
  connect (recognizer, SIGNAL (recognized (QString)),
           translator, SLOT (translate (QString)));
  QThread* translatorThread = new QThread (this);
  translator->moveToThread (translatorThread);
  translatorThread->start ();

  connect (translator, SIGNAL (translated (QString, QString)),
           SLOT (showTranslation (QString, QString)));

  trayIcon_->setContextMenu (trayContextMenu ());
  trayIcon_->show ();
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

void Manager::showTranslation(QString sourceText, QString translatedText)
{

}

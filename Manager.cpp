#include "Manager.h"

#include <QDebug>
#include <QMenu>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QThread>
#include <QSettings>
#include <QClipboard>
#include <QMessageBox>

#include "Settings.h"
#include "SettingsEditor.h"
#include "SelectionDialog.h"
#include "GlobalActionHelper.h"
#include "Recognizer.h"
#include "Translator.h"

Manager::Manager(QObject *parent) :
  QObject(parent),
  trayIcon_ (new QSystemTrayIcon (QIcon (":/images/icon.png"), this)),
  selection_ (new SelectionDialog),
  captureAction_ (NULL)
{
  GlobalActionHelper::init ();

  selection_->setWindowIcon (trayIcon_->icon ());
  connect (this, SIGNAL (showPixmap (QPixmap)),
           selection_, SLOT (setPixmap (QPixmap)));

  Recognizer* recognizer = new Recognizer;
  connect (selection_, SIGNAL (selected (QPixmap)),
           recognizer, SLOT (recognize (QPixmap)));
  connect (recognizer, SIGNAL (error (QString)),
           SLOT (showError (QString)));
  QThread* recognizerThread = new QThread (this);
  recognizer->moveToThread (recognizerThread);
  recognizerThread->start ();

  Translator* translator = new Translator;
  connect (recognizer, SIGNAL (recognized (QString)),
           translator, SLOT (translate (QString)));
  connect (translator, SIGNAL (error (QString)),
           SLOT (showError (QString)));
  QThread* translatorThread = new QThread (this);
  translator->moveToThread (translatorThread);
  translatorThread->start ();

  connect (translator, SIGNAL (translated (QString, QString)),
           SLOT (showTranslation (QString, QString)));


  connect (this, SIGNAL (settingsEdited ()), this, SLOT (applySettings ()));
  connect (this, SIGNAL (settingsEdited ()), recognizer, SLOT (applySettings ()));
  connect (this, SIGNAL (settingsEdited ()), translator, SLOT (applySettings ()));

  connect (trayIcon_, SIGNAL (activated (QSystemTrayIcon::ActivationReason)),
           SLOT (processTrayAction (QSystemTrayIcon::ActivationReason)));

  trayIcon_->setContextMenu (trayContextMenu ());
  trayIcon_->show ();

  applySettings ();
}

QMenu*Manager::trayContextMenu()
{
  QMenu* menu = new QMenu ();
  captureAction_ = menu->addAction (tr ("Захват"), this, SLOT (capture ()));
  menu->addAction (tr ("Настройки"), this, SLOT (settings ()));
  menu->addAction (tr ("О программе"), this, SLOT (about ()));
  menu->addAction (tr ("Выход"), this, SLOT (close ()));
  return menu;
}

void Manager::applySettings()
{
  QSettings settings;
  settings.beginGroup (settings_names::guiGroup);
  QString captureHotkey = settings.value (settings_names::captureHotkey,
                                          settings_values::captureHotkey).
                          toString ();
  Q_CHECK_PTR (captureAction_);
  GlobalActionHelper::removeGlobal (captureAction_);
  captureAction_->setShortcut (captureHotkey);
  GlobalActionHelper::makeGlobal (captureAction_);
}

void Manager::processTrayAction(QSystemTrayIcon::ActivationReason reason)
{
  if (reason == QSystemTrayIcon::Trigger)
  {
    if (!lastMessage_.isEmpty ())
    {
      trayIcon_->showMessage (tr ("Последний перевод"), lastMessage_,
                              QSystemTrayIcon::Information);
    }
  }
  else if (reason == QSystemTrayIcon::MiddleClick)
  {
    if (!lastMessage_.isEmpty ())
    {
      QClipboard* clipboard = QApplication::clipboard ();
      clipboard->setText (lastMessage_);
      trayIcon_->showMessage (tr ("Последний перевод"),
                              tr ("Последний перевод был скопирован в буфер обмена."),
                              QSystemTrayIcon::Information);
    }
  }
}

Manager::~Manager()
{
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
  connect (&editor, SIGNAL (settingsEdited ()), SIGNAL (settingsEdited ()));
  editor.exec ();
}

void Manager::close()
{
  QApplication::quit ();
}

void Manager::about()
{
  QString text = tr ("Программа для распознавания текста на экране.\n"\
                     "Создана с использованием Qt, tesseract-ocr, Google Translate.\n"
                     "Автор: Gres (onemoregres@gmail.com)");

  QMessageBox message (QMessageBox::Information, tr ("О программе"), text,
                       QMessageBox::Ok);
  message.setIconPixmap (trayIcon_->icon ().pixmap (QSize (64, 64)));
  message.exec ();
}

void Manager::showTranslation(QString sourceText, QString translatedText)
{
  lastMessage_ = sourceText + " - " + translatedText;
  qDebug () << sourceText << translatedText;
  trayIcon_->showMessage (tr ("Перевод"), lastMessage_, QSystemTrayIcon::Information);
}

void Manager::showError(QString text)
{
  qCritical () << text;
  trayIcon_->showMessage (tr ("Ошибка"), text, QSystemTrayIcon::Critical);
}

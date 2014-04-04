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
#include "ResultDialog.h"
#include "LanguageHelper.h"

Manager::Manager(QObject *parent) :
  QObject(parent),
  trayIcon_ (new QSystemTrayIcon (QIcon (":/images/icon.png"), this)),
  dictionary_ (new LanguageHelper),
  selection_ (new SelectionDialog (*dictionary_)),
  resultDialog_ (new ResultDialog),
  captureAction_ (NULL), repeatAction_ (NULL), clipboardAction_ (NULL),
  useResultDialog_ (true)
{
  GlobalActionHelper::init ();
  qRegisterMetaType<ProcessingItem>();

  // Recognizer
  Recognizer* recognizer = new Recognizer;
  connect (selection_, SIGNAL (selected (ProcessingItem)),
           recognizer, SLOT (recognize (ProcessingItem)));
  connect (recognizer, SIGNAL (error (QString)),
           SLOT (showError (QString)));
  connect (this, SIGNAL (settingsEdited ()),
           recognizer, SLOT (applySettings ()));
  QThread* recognizerThread = new QThread (this);
  recognizer->moveToThread (recognizerThread);
  recognizerThread->start ();


  // Translator
  Translator* translator = new Translator;
  connect (recognizer, SIGNAL (recognized (ProcessingItem)),
           translator, SLOT (translate (ProcessingItem)));
  connect (translator, SIGNAL (error (QString)),
           SLOT (showError (QString)));
  connect (this, SIGNAL (settingsEdited ()),
           translator, SLOT (applySettings ()));
  QThread* translatorThread = new QThread (this);
  translator->moveToThread (translatorThread);
  translatorThread->start ();

  connect (translator, SIGNAL (translated (ProcessingItem)),
           SLOT (showResult (ProcessingItem)));

  connect (this, SIGNAL (showPixmap (QPixmap)),
           selection_, SLOT (setPixmap (QPixmap)));

  connect (this, SIGNAL (settingsEdited ()), selection_, SLOT (updateMenu ()));
  connect (this, SIGNAL (settingsEdited ()), this, SLOT (applySettings ()));
  selection_->setWindowIcon (trayIcon_->icon ());
  resultDialog_->setWindowIcon (trayIcon_->icon ());


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
  QMenu* translateMenu = menu->addMenu (tr ("Перевод"));
  repeatAction_ = translateMenu->addAction (tr ("Повторить"), this,
                                            SLOT (showLast ()));
  clipboardAction_ = translateMenu->addAction (tr ("Скопировать"), this,
                                               SLOT (copyLastToClipboard ()));
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
                                          settings_values::captureHotkey).toString ();
  Q_CHECK_PTR (captureAction_);
  GlobalActionHelper::removeGlobal (captureAction_);
  captureAction_->setShortcut (captureHotkey);
  GlobalActionHelper::makeGlobal (captureAction_);

  QString repeatHotkey = settings.value (settings_names::repeatHotkey,
                                         settings_values::repeatHotkey).toString ();
  Q_CHECK_PTR (repeatAction_);
  GlobalActionHelper::removeGlobal (repeatAction_);
  repeatAction_->setShortcut (repeatHotkey);
  GlobalActionHelper::makeGlobal (repeatAction_);

  QString clipboardHotkey = settings.value (settings_names::clipboardHotkey,
                                            settings_values::clipboardHotkey).toString ();
  Q_CHECK_PTR (clipboardAction_);
  GlobalActionHelper::removeGlobal (clipboardAction_);
  clipboardAction_->setShortcut (clipboardHotkey);
  GlobalActionHelper::makeGlobal (clipboardAction_);

  // Depends on SettingsEditor button indexes. 1==dialog
  useResultDialog_ = settings.value (settings_names::resultShowType,
                                     settings_values::resultShowType).toBool ();

  Q_CHECK_PTR (dictionary_);
  dictionary_->updateAvailableOcrLanguages ();
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
  SettingsEditor editor (*dictionary_);
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
                     "Автор: Gres (translator@gres.biz)");

  QMessageBox message (QMessageBox::Information, tr ("О программе"), text,
                       QMessageBox::Ok);
  message.setIconPixmap (trayIcon_->icon ().pixmap (QSize (64, 64)));
  message.exec ();
}

void Manager::processTrayAction(QSystemTrayIcon::ActivationReason reason)
{
  if (reason == QSystemTrayIcon::Trigger)
  {
    showLast ();
  }
  else if (reason == QSystemTrayIcon::MiddleClick)
  {
    copyLastToClipboard  ();
  }
}

void Manager::showLast()
{
  if (lastItem_.isValid ())
  {
    showResult (lastItem_);
  }
}

void Manager::copyLastToClipboard()
{
  if (lastItem_.isValid ())
  {
    QClipboard* clipboard = QApplication::clipboard ();
    QString message = lastItem_.recognized + " - " + lastItem_.translated;
    clipboard->setText (message);
    trayIcon_->showMessage (tr ("Перевод"),
                            tr ("Последний перевод был скопирован в буфер обмена."),
                            QSystemTrayIcon::Information);
  }
}

void Manager::showResult(ProcessingItem item)
{
  Q_ASSERT (item.isValid ());
  lastItem_ = item;
  if (useResultDialog_)
  {
    resultDialog_->showResult (item);
  }
  else
  {
    QString message = item.recognized + " - " + item.translated;
    trayIcon_->showMessage (tr ("Перевод"), message, QSystemTrayIcon::Information);
  }
}

void Manager::showError(QString text)
{
  qCritical () << text;
  trayIcon_->showMessage (tr ("Ошибка"), text, QSystemTrayIcon::Critical);
}

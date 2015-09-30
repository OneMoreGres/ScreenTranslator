#include "Manager.h"

#include <QDebug>
#include <QMenu>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QDesktopWidget>
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
#include "StAssert.h"

Manager::Manager (QObject *parent) :
  QObject (parent),
  trayIcon_ (new QSystemTrayIcon (QIcon (":/images/icon.png"), this)),
  dictionary_ (new LanguageHelper),
  resultDialog_ (new ResultDialog),
  captureAction_ (NULL), repeatCaptureAction_ (NULL),
  repeatAction_ (NULL), clipboardAction_ (NULL),
  useResultDialog_ (true) {
  GlobalActionHelper::init ();
  qRegisterMetaType<ProcessingItem>();

  // Recognizer
  Recognizer *recognizer = new Recognizer;
  connect (this, SIGNAL (requestRecognize (ProcessingItem)),
           recognizer, SLOT (recognize (ProcessingItem)));
  connect (recognizer, SIGNAL (recognized (ProcessingItem)),
           SIGNAL (recognized (ProcessingItem)));
  connect (recognizer, SIGNAL (error (QString)),
           SLOT (showError (QString)));
  connect (this, SIGNAL (settingsEdited ()),
           recognizer, SLOT (applySettings ()));
  QThread *recognizerThread = new QThread (this);
  threads_ << recognizerThread;
  recognizer->moveToThread (recognizerThread);
  recognizerThread->start ();
  connect (qApp, SIGNAL (aboutToQuit ()), recognizerThread, SLOT (quit ()));


  // Translator
  Translator *translator = new Translator;
  connect (this, SIGNAL (requestTranslate (ProcessingItem)),
           translator, SLOT (translate (ProcessingItem)));
  connect (translator, SIGNAL (translated (ProcessingItem)),
           SLOT (showResult (ProcessingItem)));
  connect (translator, SIGNAL (error (QString)),
           SLOT (showError (QString)));
  connect (this, SIGNAL (settingsEdited ()),
           translator, SLOT (applySettings ()));
  QThread *translatorThread = new QThread (this);
  threads_ << translatorThread;
  translator->moveToThread (translatorThread);
  translatorThread->start ();
  connect (qApp, SIGNAL (aboutToQuit ()), translatorThread, SLOT (quit ()));

  connect (this, SIGNAL (settingsEdited ()), this, SLOT (applySettings ()));
  resultDialog_->setWindowIcon (trayIcon_->icon ());


  connect (trayIcon_, SIGNAL (activated (QSystemTrayIcon::ActivationReason)),
           SLOT (processTrayAction (QSystemTrayIcon::ActivationReason)));

  trayIcon_->setContextMenu (trayContextMenu ());
  updateActionsState ();
  trayIcon_->show ();

  applySettings ();
}

QMenu * Manager::trayContextMenu () {
  QMenu *menu = new QMenu ();
  captureAction_ = menu->addAction (tr ("Захват"), this, SLOT (capture ()));
  repeatCaptureAction_ = menu->addAction (tr ("Повторить захват"),
                                          this, SLOT (repeatCapture ()));
  QMenu *translateMenu = menu->addMenu (tr ("Результат"));
  repeatAction_ = translateMenu->addAction (tr ("Показать"), this,
                                            SLOT (showLast ()));
  clipboardAction_ = translateMenu->addAction (tr ("В буфер"), this,
                                               SLOT (copyLastToClipboard ()));
  menu->addAction (tr ("Настройки"), this, SLOT (settings ()));
  menu->addAction (tr ("О программе"), this, SLOT (about ()));
  menu->addAction (tr ("Выход"), this, SLOT (close ()));
  return menu;
}

void Manager::updateActionsState (bool isEnabled) {
#ifdef Q_OS_LINUX
  // Avoid unneeded tray blinking (required to update context menu).
  QList<QAction *> actions;
  actions << captureAction_ << repeatCaptureAction_ << repeatAction_ << clipboardAction_;
  QList<bool> states;
  foreach (const QAction * action, actions) {
    states << action->isEnabled ();
  }
#endif
  captureAction_->setEnabled (isEnabled);
  repeatCaptureAction_->setEnabled (isEnabled && !selections_.isEmpty ());
  repeatAction_->setEnabled (isEnabled && lastItem_.isValid ());
  clipboardAction_->setEnabled (isEnabled && lastItem_.isValid ());
#ifdef Q_OS_LINUX
  for (int i = 0, end = actions.size (); i < end; ++i) {
    if (states.at (i) != actions.at (i)->isEnabled ()) {
      trayIcon_->hide ();
      trayIcon_->show ();
      break;
    }
  }
#endif
}

void Manager::applySettings () {
#define GET(NAME) settings.value (settings_names::NAME, settings_values::NAME)
  QSettings settings;
  settings.beginGroup (settings_names::guiGroup);

  Q_CHECK_PTR (captureAction_);
  GlobalActionHelper::removeGlobal (captureAction_);
  captureAction_->setShortcut (GET (captureHotkey).toString ());
  GlobalActionHelper::makeGlobal (captureAction_);

  Q_CHECK_PTR (repeatCaptureAction_);
  GlobalActionHelper::removeGlobal (repeatCaptureAction_);
  repeatCaptureAction_->setShortcut (GET (repeatCaptureHotkey).toString ());
  GlobalActionHelper::makeGlobal (repeatCaptureAction_);

  Q_CHECK_PTR (repeatAction_);
  GlobalActionHelper::removeGlobal (repeatAction_);
  repeatAction_->setShortcut (GET (repeatHotkey).toString ());
  GlobalActionHelper::makeGlobal (repeatAction_);

  Q_CHECK_PTR (clipboardAction_);
  GlobalActionHelper::removeGlobal (clipboardAction_);
  clipboardAction_->setShortcut (GET (clipboardHotkey).toString ());
  GlobalActionHelper::makeGlobal (clipboardAction_);

  // Depends on SettingsEditor button indexes. 1==dialog
  useResultDialog_ = GET (resultShowType).toBool ();
  settings.endGroup ();

  Q_CHECK_PTR (dictionary_);
  dictionary_->updateAvailableOcrLanguages ();

  settings.beginGroup (settings_names::translationGroup);
  bool doTranslation = GET (doTranslation).toBool ();
  if (doTranslation) {
    disconnect (this, SIGNAL (recognized (ProcessingItem)),
                this, SLOT (showResult (ProcessingItem)));
    connect (this, SIGNAL (recognized (ProcessingItem)),
             this, SIGNAL (requestTranslate (ProcessingItem)), Qt::UniqueConnection);
  }
  else {
    disconnect (this, SIGNAL (recognized (ProcessingItem)),
                this, SIGNAL (requestTranslate (ProcessingItem)));
    connect (this, SIGNAL (recognized (ProcessingItem)),
             this, SLOT (showResult (ProcessingItem)), Qt::UniqueConnection);
  }
#undef GET
}

Manager::~Manager () {
  foreach (QThread * thread, threads_) {
    thread->quit ();
    thread->wait (1000000);
  }
}

void Manager::capture () {
  QList<QScreen *> screens = QApplication::screens ();
  foreach (QScreen * screen, screens) {
    QRect geometry = screen->availableGeometry ();
    QPixmap pixmap = screen->grabWindow (0, geometry.x (), geometry.y (),
                                         geometry.width (), geometry.height ());
    QString name = screen->name ();
    if (!selections_.contains (name)) {
      SelectionDialog *selection = new SelectionDialog (*dictionary_);
      selection->setWindowIcon (trayIcon_->icon ());
      connect (this, SIGNAL (closeSelections ()), selection, SLOT (close ()));
      connect (this, SIGNAL (settingsEdited ()), selection, SLOT (updateMenu ()));
      connect (selection, SIGNAL (selected (ProcessingItem)),
               SLOT (handleSelection (ProcessingItem)));
      connect (selection, SIGNAL (rejected ()), SIGNAL (closeSelections ()));
      selections_[name] = selection;
    }
    SelectionDialog *selection = selections_[name];
    selection->setPixmap (pixmap, geometry);
  }
  updateActionsState ();
}

void Manager::handleSelection (ProcessingItem item) {
  emit requestRecognize (item);
  if (!(item.modifiers & Qt::ControlModifier)) {
    emit closeSelections ();
  }
}

void Manager::repeatCapture () {
  if (selections_.isEmpty ()) {
    return;
  }
  QList<QScreen *> screens = QApplication::screens ();
  foreach (QScreen * screen, screens) {
    QString name = screen->name ();
    if (!selections_.contains (name)) {
      continue;
    }
    SelectionDialog *selection = selections_[name];
    selection->show ();
    selection->activateWindow ();
  }
}

void Manager::settings () {
  SettingsEditor editor (*dictionary_);
  editor.setWindowIcon (trayIcon_->icon ());
  connect (&editor, SIGNAL (settingsEdited ()), SIGNAL (settingsEdited ()));
  updateActionsState (false);
  editor.exec ();
  updateActionsState (true);
}

void Manager::close () {
  QApplication::quit ();
}

void Manager::about () {
  QString version = "1.2.3";
  QString text = tr ("Программа для распознавания текста на экране.\n" \
                     "Создана с использованием Qt, tesseract-ocr, Google Translate.\n"
                     "Автор: Gres (translator@gres.biz)\n"
                     "Версия: %1 от %2 %3").arg (version)
                 .arg (__DATE__).arg (__TIME__);

  QMessageBox message (QMessageBox::Information, tr ("О программе"), text,
                       QMessageBox::Ok);
  message.setIconPixmap (trayIcon_->icon ().pixmap (QSize (64, 64)));
  message.exec ();
}

void Manager::processTrayAction (QSystemTrayIcon::ActivationReason reason) {
  if (reason == QSystemTrayIcon::Trigger && repeatAction_->isEnabled ()) {
    showLast ();
  }
  else if (reason == QSystemTrayIcon::MiddleClick && clipboardAction_->isEnabled ()) {
    copyLastToClipboard  ();
  }
  else if (reason == QSystemTrayIcon::DoubleClick && repeatCaptureAction_->isEnabled ()) {
    repeatCapture ();
  }
}

void Manager::showLast () {
  if (lastItem_.isValid ()) {
    showResult (lastItem_);
  }
}

void Manager::copyLastToClipboard () {
  if (lastItem_.isValid ()) {
    QClipboard *clipboard = QApplication::clipboard ();
    QString message = lastItem_.recognized;
    if (!lastItem_.translated.isEmpty ()) {
      message += " - " + lastItem_.translated;
    }
    clipboard->setText (message);
    trayIcon_->showMessage (tr ("Результат"),
                            tr ("Последний результат был скопирован в буфер обмена."),
                            QSystemTrayIcon::Information);
  }
}

void Manager::showResult (ProcessingItem item) {
  ST_ASSERT (item.isValid ());
  lastItem_ = item;
  if (useResultDialog_) {
    resultDialog_->showResult (item);
  }
  else {
    QString message = item.recognized + " - " + item.translated;
    trayIcon_->showMessage (tr ("Результат"), message, QSystemTrayIcon::Information);
  }
  updateActionsState ();
}

void Manager::showError (QString text) {
  qCritical () << text;
  trayIcon_->showMessage (tr ("Ошибка"), text, QSystemTrayIcon::Critical);
}

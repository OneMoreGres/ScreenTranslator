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
#include <QInputDialog>
#include <QNetworkProxy>

#include "Settings.h"
#include "SettingsEditor.h"
#include "SelectionDialog.h"
#include "GlobalActionHelper.h"
#include "Recognizer.h"
#include "WebTranslator.h"
#include "ResultDialog.h"
#include "LanguageHelper.h"
#include "StAssert.h"
#include "Utils.h"

Manager::Manager (QObject *parent) :
  QObject (parent),
  trayIcon_ (new QSystemTrayIcon (this)),
  dictionary_ (new LanguageHelper),
  resultDialog_ (new ResultDialog (*dictionary_)),
  captureAction_ (NULL), repeatCaptureAction_ (NULL),
  repeatAction_ (NULL), clipboardAction_ (NULL),
  useResultDialog_ (true), doTranslation_ (true), itemProcessingCount_ (0) {
  updateNormalIcon ();
  GlobalActionHelper::init ();
  qRegisterMetaType<ProcessingItem>();

  // Recognizer
  Recognizer *recognizer = new Recognizer;
  connect (this, SIGNAL (requestRecognize (ProcessingItem)),
           recognizer, SLOT (recognize (ProcessingItem)));
  connect (recognizer, SIGNAL (recognized (ProcessingItem)),
           this, SIGNAL (requestTranslate (ProcessingItem)));
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
  WebTranslator *translator = new WebTranslator;
  connect (this, SIGNAL (requestTranslate (ProcessingItem)),
           translator, SLOT (translate (ProcessingItem)));
  connect (translator, SIGNAL (translated (ProcessingItem)),
           SLOT (showResult (ProcessingItem)));
  connect (translator, SIGNAL (error (QString)),
           SLOT (showError (QString)));
  connect (this, SIGNAL (settingsEdited ()),
           translator, SLOT (applySettings ()));

  connect (this, SIGNAL (settingsEdited ()), this, SLOT (applySettings ()));

  resultDialog_->setWindowIcon (trayIcon_->icon ());
  connect (this, SIGNAL (settingsEdited ()), resultDialog_, SLOT (applySettings ()));
  connect (resultDialog_, SIGNAL (requestRecognize (ProcessingItem)),
           this, SIGNAL (requestRecognize (ProcessingItem)));
  connect (resultDialog_, SIGNAL (requestTranslate (ProcessingItem)),
           this, SIGNAL (requestTranslate (ProcessingItem)));
  connect (resultDialog_, SIGNAL (requestClipboard ()), SLOT (copyLastToClipboard ()));
  connect (resultDialog_, SIGNAL (requestImageClipboard ()),
           SLOT (copyLastImageToClipboard ()));
  connect (resultDialog_, SIGNAL (requestEdition (ProcessingItem)),
           this, SLOT (editRecognized (ProcessingItem)));


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
  const ProcessingItem &lastItem = resultDialog_->item ();
  repeatAction_->setEnabled (isEnabled && lastItem.isValid ());
  clipboardAction_->setEnabled (isEnabled && lastItem.isValid ());
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

  QNetworkProxy proxy = QNetworkProxy::applicationProxy ();
  proxy.setType (QNetworkProxy::ProxyType (GET (proxyType).toInt ()));
  proxy.setHostName (GET (proxyHostName).toString ());
  proxy.setPort (GET (proxyPort).toInt ());
  proxy.setUser (GET (proxyUser).toString ());
  if (GET (proxySavePassword).toBool ()) {
    proxy.setPassword (encode (GET (proxyPassword).toString ()));
  }
  QNetworkProxy::setApplicationProxy (proxy);
  settings.endGroup ();

  settings.beginGroup (settings_names::recogntionGroup);
  defaultOrcLanguage_ = GET (ocrLanguage).toString ();
  settings.endGroup ();

  settings.beginGroup (settings_names::translationGroup);
  defaultTranslationLanguage_ = GET (translationLanguage).toString ();
  doTranslation_ = GET (doTranslation).toBool ();
  settings.endGroup ();

  Q_CHECK_PTR (dictionary_);
  dictionary_->updateAvailableOcrLanguages ();
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
      connect (this, SIGNAL (settingsEdited ()), selection, SLOT (applySettings ()));
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
  bool altMod = item.modifiers & Qt::AltModifier;
  bool doTranslation = (doTranslation_ && !altMod) || (!doTranslation_ && altMod);
  if (doTranslation) {
    item.translateLanguage = defaultTranslationLanguage_;
  }
  if (item.ocrLanguage.isEmpty ()) {
    item.ocrLanguage = defaultOrcLanguage_;
  }
  if (item.swapLanguages_) {
    QString translate = (item.translateLanguage.isEmpty ())
                        ? defaultTranslationLanguage_ : item.translateLanguage;
    if (doTranslation) {
      item.translateLanguage = dictionary_->ocrToTranslateCodes (item.ocrLanguage);
    }
    item.sourceLanguage.clear ();
    item.ocrLanguage = dictionary_->translateToOcrCodes (translate);
    if (item.ocrLanguage.isEmpty ()) {
      showError (tr ("Не найден подходящий язык распознавания."));
      return;
    }
  }
  if (item.sourceLanguage.isEmpty ()) {
    item.sourceLanguage = dictionary_->ocrToTranslateCodes (item.ocrLanguage);
  }
  emit requestRecognize (item);
  ++itemProcessingCount_;
  updateNormalIcon ();
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
  QString tips = tr ("\n\nПодсказки.\n"
                     "Клик по иконке в трее:\n"
                     "* левой кнопкой - отобразить последний результат\n"
                     "* средней кнопкой - скопировать последний результат в буфер обмена\n"
#ifdef Q_OS_WIN
                     "* двойной клик - повторный захват последнего экрана\n"
#endif
                     "\n"
                     "Захвата изображения при зажатых кнопках:\n"
                     "* Ctrl - не выходить из режима захвата\n"
                     "* Alt - выполнить перевод, если в настройках он выключен "
                     "(и наоборот, не выполнять, если включен)\n"
                     "");

  QMessageBox message (QMessageBox::Information, tr ("О программе"), text + tips,
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
    trayIcon_->showMessage (tr ("Результат"),
                            tr ("Последний результат был скопирован в буфер обмена."),
                            QSystemTrayIcon::Information);
  }
  else if (reason == QSystemTrayIcon::DoubleClick && repeatCaptureAction_->isEnabled ()) {
    repeatCapture ();
  }
}

void Manager::editRecognized (ProcessingItem item) {
  QString fixed = QInputDialog::getMultiLineText (
    NULL, tr ("Правка"), tr ("Исправьте распознанный текст"), item.recognized);
  if (!fixed.isEmpty ()) {
    item.recognized = fixed;
    emit requestTranslate (item);
  }
}

void Manager::showLast () {
  const ProcessingItem &item = resultDialog_->item ();
  if (item.isValid ()) {
    showResult (item);
  }
}

void Manager::copyLastToClipboard () {
  const ProcessingItem &item = resultDialog_->item ();
  if (item.isValid ()) {
    QClipboard *clipboard = QApplication::clipboard ();
    QString message = item.recognized;
    if (!item.translated.isEmpty ()) {
      message += " - " + item.translated;
    }
    clipboard->setText (message);
  }
}

void Manager::copyLastImageToClipboard () {
  const ProcessingItem &item = resultDialog_->item ();
  if (item.isValid ()) {
    QClipboard *clipboard = QApplication::clipboard ();
    clipboard->setPixmap (item.source);
  }
}

void Manager::showResult (ProcessingItem item) {
  --itemProcessingCount_;
  if (!item.isValid ()) {
    // delay because it can show error
    QTimer::singleShot (3000, this, SLOT (updateNormalIcon ()));
    return;
  }
  changeIcon (IconTypeSuccess);
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
  changeIcon (IconTypeError);
  trayIcon_->showMessage (tr ("Ошибка"), text, QSystemTrayIcon::Critical);
}

void Manager::changeIcon (int iconType, int timeoutMsec) {
  QString fileName;
  switch (iconType) {
    case IconTypeSuccess:
      fileName = ":/images/STIconGreen.png";
      break;
    case IconTypeError:
      fileName = ":/images/STIconRed.png";
      break;
    default:
      return;
  }
  trayIcon_->setIcon (QIcon (fileName));
  if (timeoutMsec > 0) {
    QTimer::singleShot (timeoutMsec, this, SLOT (updateNormalIcon ()));
  }
}

void Manager::updateNormalIcon () {
  QString fileName = itemProcessingCount_ > 0
                     ? ":/images/STIconOrange.png" : ":/images/STIconBlue.png";
  trayIcon_->setIcon (QIcon (fileName));
}

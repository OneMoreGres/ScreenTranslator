#include "SettingsEditor.h"
#include "ui_SettingsEditor.h"
#include "LanguageHelper.h"

#include <QSettings>
#include <QFileDialog>

#include "Settings.h"

SettingsEditor::SettingsEditor (const LanguageHelper &dictionary, QWidget *parent) :
  QDialog (parent),
  ui (new Ui::SettingsEditor), dictionary_ (dictionary),
  buttonGroup_ (new QButtonGroup (this)) {
  ui->setupUi (this);

  buttonGroup_->addButton (ui->trayRadio, 0);
  buttonGroup_->addButton (ui->dialogRadio, 1);

  connect (ui->tessdataButton, SIGNAL (clicked ()), SLOT (openTessdataDialog ()));
  connect (ui->tessdataEdit, SIGNAL (textChanged (const QString &)),
           SLOT (initOcrLangCombo (const QString &)));

  ui->translateLangCombo->addItems (dictionary_.translateLanguagesUi ());
  loadSettings ();
  loadState ();
}

SettingsEditor::~SettingsEditor () {
  saveState ();
  delete ui;
}

void SettingsEditor::done (int result) {
  if (result == QDialog::Accepted) {
    saveSettings ();
    emit settingsEdited ();
  }
  QDialog::done (result);
}

void SettingsEditor::saveSettings () const {
  using namespace settings_names;
  QSettings settings;
  settings.beginGroup (guiGroup);
  settings.setValue (captureHotkey, ui->captureEdit->keySequence ().toString ());
  settings.setValue (repeatCaptureHotkey, ui->repeatCaptureEdit->keySequence ().toString ());
  settings.setValue (repeatHotkey, ui->repeatEdit->keySequence ().toString ());
  settings.setValue (clipboardHotkey, ui->clipboardEdit->keySequence ().toString ());
  settings.setValue (resultShowType, buttonGroup_->checkedId ());
  settings.endGroup ();


  settings.beginGroup (recogntionGroup);
  settings.setValue (tessDataPlace, ui->tessdataEdit->text ());
  QString ocrLanguage = dictionary_.ocrUiToCode (ui->ocrLangCombo->currentText ());
  settings.setValue (ocrLanguage, ocrLanguage);
  settings.setValue (imageScale, ui->imageScaleSpin->value ());
  settings.endGroup ();


  settings.beginGroup (translationGroup);
  settings.setValue (doTranslation, ui->doTranslationCombo->isChecked ());
  QString trLanguage = dictionary_.translateUiToCode (ui->translateLangCombo->currentText ());
  settings.setValue (translationLanguage, trLanguage);
  QString sourceLanguage = dictionary_.translateForOcrCode (ocrLanguage);
  settings.setValue (sourceLanguage, sourceLanguage);
  settings.endGroup ();
}

void SettingsEditor::openTessdataDialog () {
  QString path = QFileDialog::getExistingDirectory (this, tr ("Путь к tessdata"));
  if (path.isEmpty ()) {
    return;
  }
  QDir dir (path);
  if (dir.dirName () == QString ("tessdata")) {
    dir.cdUp ();
  }
  ui->tessdataEdit->setText (dir.path ());
}

void SettingsEditor::loadSettings () {
#define GET(FIELD) settings.value (settings_names::FIELD, settings_values::FIELD)
  QSettings settings;

  settings.beginGroup (settings_names::guiGroup);
  ui->captureEdit->setKeySequence (QKeySequence (GET (captureHotkey).toString ()));
  ui->repeatCaptureEdit->setKeySequence (QKeySequence (GET (repeatCaptureHotkey).toString ()));
  ui->repeatEdit->setKeySequence (QKeySequence (GET (repeatHotkey).toString ()));
  ui->clipboardEdit->setKeySequence (QKeySequence (GET (clipboardHotkey).toString ()));
  QAbstractButton *button = buttonGroup_->button (GET (resultShowType).toInt ());
  Q_CHECK_PTR (button);
  button->setChecked (true);
  settings.endGroup ();

  settings.beginGroup (settings_names::recogntionGroup);
  ui->tessdataEdit->setText (GET (tessDataPlace).toString ());
  QString ocrLanguage = dictionary_.ocrCodeToUi (GET (ocrLanguage).toString ());
  ui->ocrLangCombo->setCurrentText (ocrLanguage);
  ui->imageScaleSpin->setValue (GET (imageScale).toInt ());
  settings.endGroup ();

  settings.beginGroup (settings_names::translationGroup);
  ui->doTranslationCombo->setChecked (GET (doTranslation).toBool ());
  QString trLanguage = dictionary_.translateCodeToUi (GET (translationLanguage).toString ());
  ui->translateLangCombo->setCurrentText (trLanguage);
  settings.endGroup ();
#undef GET
}

void SettingsEditor::saveState () const {
  QSettings settings;
  settings.beginGroup (settings_names::guiGroup);
  settings.setValue (objectName () + "_" + settings_names::geometry, saveGeometry ());
}

void SettingsEditor::loadState () {
  QSettings settings;
  settings.beginGroup (settings_names::guiGroup);
  restoreGeometry (settings.value (objectName () + "_" + settings_names::geometry).toByteArray ());
}

void SettingsEditor::initOcrLangCombo (const QString &path) {
  ui->ocrLangCombo->clear ();
  ui->ocrLangCombo->addItems (dictionary_.availableOcrLanguagesUi (path));
}

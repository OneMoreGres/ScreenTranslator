#include "SettingsEditor.h"
#include "ui_SettingsEditor.h"
#include "LanguageHelper.h"
#include "TranslatorHelper.h"
#include "RecognizerHelper.h"
#include "StAssert.h"

#include <QSettings>
#include <QFileDialog>
#include <QDir>

#include "Settings.h"

SettingsEditor::SettingsEditor (const LanguageHelper &dictionary, QWidget *parent) :
  QDialog (parent),
  ui (new Ui::SettingsEditor), translatorHelper_ (new TranslatorHelper),
  recognizerHelper_ (new RecognizerHelper), dictionary_ (dictionary),
  buttonGroup_ (new QButtonGroup (this)) {
  ui->setupUi (this);

  buttonGroup_->addButton (ui->trayRadio, 0);
  buttonGroup_->addButton (ui->dialogRadio, 1);

  connect (ui->tessdataButton, SIGNAL (clicked ()), SLOT (openTessdataDialog ()));
  connect (ui->tessdataEdit, SIGNAL (textChanged (const QString &)),
           SLOT (initOcrLangCombo (const QString &)));

  connect (ui->recognizerFixTable, SIGNAL (itemChanged (QTableWidgetItem *)),
           SLOT (recognizerFixTableItemChanged (QTableWidgetItem *)));

  ui->translateLangCombo->addItems (dictionary_.translateLanguagesUi ());
  loadSettings ();
  loadState ();
}

SettingsEditor::~SettingsEditor () {
  saveState ();
  delete recognizerHelper_;
  delete translatorHelper_;
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
  QString ocrLanguageVal = dictionary_.ocrUiToCode (ui->ocrLangCombo->currentText ());
  settings.setValue (ocrLanguage, ocrLanguageVal);
  settings.setValue (imageScale, ui->imageScaleSpin->value ());

  {  //Recognizer substitutions
    RecognizerHelper::Subs subs;
    QTableWidget *t = ui->recognizerFixTable; // Shortcut
    for (int i = 0, end = t->rowCount () - 1; i < end; ++i) {
      QComboBox *combo = static_cast<QComboBox *>(t->cellWidget (i, SubsColLanguage));
      QString langUi = combo->currentText ();
      RecognizerHelper::Sub sub;
      sub.language = dictionary_.ocrUiToCode (langUi);
#define GET(COL) (t->item (i, COL) ? t->item (i, COL)->text () : QString ())
      sub.source = GET (SubsColSource);
      sub.target = GET (SubsColTarget);
#undef GET
      if (langUi.isEmpty () || sub.language == langUi || sub.source.isEmpty ()) {
        continue;
      }
      subs.append (sub);
    }
    recognizerHelper_->setSubs (subs);
    recognizerHelper_->save ();
  }

  settings.endGroup ();

  settings.beginGroup (translationGroup);
  settings.setValue (doTranslation, ui->doTranslationCheck->isChecked ());
  settings.setValue (translationDebugMode, ui->translatorDebugCheck->isChecked ());
  QString trLanguage = dictionary_.translateUiToCode (ui->translateLangCombo->currentText ());
  settings.setValue (translationLanguage, trLanguage);
  QString sourceLanguageVal = dictionary_.translateForOcrCode (ocrLanguage);
  settings.setValue (sourceLanguage, sourceLanguageVal);
  settings.setValue (translationTimeout, ui->translateTimeoutSpin->value ());

  {//Translators
    QStringList enabled;
    for (int i = 0, end = ui->translatorList->count (); i < end; ++i) {
      QListWidgetItem *item = ui->translatorList->item (i);
      if (item->checkState () == Qt::Checked) {
        enabled << item->text ();
      }
    }
    translatorHelper_->setEnabledTranslators (enabled);
  }

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

  {//Recognizer substitutions
    recognizerHelper_->load ();
    RecognizerHelper::Subs subs = recognizerHelper_->subs ();
    ui->recognizerFixTable->setRowCount (subs.size ());
    int row = 0;
    foreach (const RecognizerHelper::Sub & sub, subs) {
      if (!initSubsTableRow (row, sub.language)) {
        continue;
      }
      ui->recognizerFixTable->setItem (row, SubsColSource, new QTableWidgetItem (sub.source));
      ui->recognizerFixTable->setItem (row, SubsColTarget, new QTableWidgetItem (sub.target));
      ++row;
    }
    ui->recognizerFixTable->setRowCount (row + 1);
    initSubsTableRow (row);
    ui->recognizerFixTable->resizeColumnsToContents ();
  }

  settings.endGroup ();


  settings.beginGroup (settings_names::translationGroup);
  ui->doTranslationCheck->setChecked (GET (doTranslation).toBool ());
  ui->translatorDebugCheck->setChecked (GET (translationDebugMode).toBool ());
  QString trLanguage = dictionary_.translateCodeToUi (GET (translationLanguage).toString ());
  ui->translateLangCombo->setCurrentText (trLanguage);
  ui->translateTimeoutSpin->setValue (GET (translationTimeout).toInt ());

  {// Translators
    QStringList enabled;
    ui->translatorList->addItems (translatorHelper_->possibleTranslators (enabled));
    for (int i = 0, end = ui->translatorList->count (); i < end; ++i) {
      QListWidgetItem *item = ui->translatorList->item (i);
      Qt::CheckState state = enabled.contains (item->text ()) ? Qt::Checked : Qt::Unchecked;
      item->setCheckState (state);
    }
  }

  settings.endGroup ();
#undef GET
}

bool SettingsEditor::initSubsTableRow (int row, const QString &languageCode) {
  QString lang = dictionary_.ocrCodeToUi (languageCode);
  if (!languageCode.isEmpty () && lang == languageCode) {
    return false;
  }
  QComboBox *langCombo = new QComboBox (ui->recognizerFixTable);
  langCombo->setModel (ui->ocrLangCombo->model ());
  if (!languageCode.isEmpty ()) {
    langCombo->setCurrentText (lang);
  }
  else {
    langCombo->setCurrentIndex (ui->ocrLangCombo->currentIndex ());
  }
  ui->recognizerFixTable->setCellWidget (row, SubsColLanguage, langCombo);
  return true;
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

void SettingsEditor::recognizerFixTableItemChanged (QTableWidgetItem *item) {
  ST_ASSERT (item->column () < 3);
  int row = item->row ();
  QTableWidget *t = ui->recognizerFixTable;
#define CHECK(COL) (!t->item (row, COL) || t->item (row, COL)->text ().isEmpty ())
  bool isRowEmpty = CHECK (SubsColSource) && CHECK (SubsColTarget);
#undef CHECK
  int lastRow = ui->recognizerFixTable->rowCount () - 1;
  if (isRowEmpty && row != lastRow) {
    ui->recognizerFixTable->removeRow (row);
  }
  else if (!isRowEmpty && row == lastRow) {
    int newRow = lastRow + 1;
    ui->recognizerFixTable->insertRow (newRow);
    initSubsTableRow (newRow);
  }
}

#include "SettingsEditor.h"
#include "ui_SettingsEditor.h"

#include <QSettings>
#include <QFileDialog>

#include "Settings.h"



SettingsEditor::SettingsEditor(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SettingsEditor),
  buttonGroup_ (new QButtonGroup (this))
{
  ui->setupUi(this);

  buttonGroup_->addButton (ui->trayRadio, 0);
  buttonGroup_->addButton (ui->dialogRadio, 1);

  connect (ui->tessdataButton, SIGNAL (clicked ()), SLOT (openTessdataDialog ()));
  connect (ui->tessdataEdit, SIGNAL (textChanged (const QString&)),
           SLOT (initOcrLangCombo ()));

  initTranslateLangCombo ();
  loadSettings ();
  loadState ();
}

SettingsEditor::~SettingsEditor()
{
  saveState ();
  delete ui;
}

void SettingsEditor::done(int result)
{
  if (result == QDialog::Accepted)
  {
    saveSettings ();
    emit settingsEdited ();
  }
  QDialog::done (result);
}

void SettingsEditor::saveSettings() const
{
  QSettings settings;
  settings.beginGroup (settings_names::guiGroup);
  settings.setValue (settings_names::captureHotkey, ui->captureEdit->text ());
  settings.setValue (settings_names::repeatHotkey, ui->repeatEdit->text ());
  settings.setValue (settings_names::clipboardHotkey, ui->clipboardEdit->text ());
  settings.setValue (settings_names::resultShowType, buttonGroup_->checkedId ());
  settings.endGroup ();


  settings.beginGroup (settings_names::recogntionGroup);
  settings.setValue (settings_names::tessDataPlace, ui->tessdataEdit->text ());
  settings.setValue (settings_names::ocrLanguage, ui->ocrLangCombo->currentText ());
  settings.setValue (settings_names::imageScale, ui->imageScaleSpin->value ());
  settings.endGroup ();


  settings.beginGroup (settings_names::translationGroup);
  settings.setValue (settings_names::translationLanguage,
                     ui->translateLangCombo->currentText ());
  settings.endGroup ();
}

void SettingsEditor::openTessdataDialog()
{
  QString path = QFileDialog::getExistingDirectory (this, tr ("Путь к tessdata"));
  if (path.isEmpty ())
  {
    return;
  }
  QDir dir (path);
  if (dir.dirName () == QString ("tessdata"))
  {
    dir.cdUp ();
  }
  ui->tessdataEdit->setText (dir.path ());
}

void SettingsEditor::loadSettings()
{
  QSettings settings;
  settings.beginGroup (settings_names::guiGroup);
  QString captureHotkey = settings.value (settings_names::captureHotkey,
                                          settings_values::captureHotkey).toString ();
  ui->captureEdit->setText (captureHotkey);
  QString repeatHotkey = settings.value (settings_names::repeatHotkey,
                                         settings_values::repeatHotkey).toString ();
  ui->repeatEdit->setText (repeatHotkey);
  QString clipboardHotkey = settings.value (settings_names::clipboardHotkey,
                                            settings_values::clipboardHotkey).toString ();
  ui->clipboardEdit->setText (clipboardHotkey);
  int resultShowType = settings.value (settings_names::resultShowType,
                                       settings_values::resultShowType).toInt ();
  QAbstractButton* button = buttonGroup_->button (resultShowType);
  Q_CHECK_PTR (button);
  button->setChecked (true);
  settings.endGroup ();


  settings.beginGroup (settings_names::recogntionGroup);
  QString tessDataPlace = settings.value (settings_names::tessDataPlace,
                                          settings_values::tessDataPlace).toString ();
  ui->tessdataEdit->setText (tessDataPlace);
  QString ocrLang = settings.value (settings_names::ocrLanguage,
                                    settings_values::ocrLanguage).toString ();
  ui->ocrLangCombo->setCurrentText (ocrLang);
  int imageScale = settings.value (settings_names::imageScale,
                                   settings_values::imageScale).toInt ();
  ui->imageScaleSpin->setValue (imageScale);
  settings.endGroup ();


  settings.beginGroup (settings_names::translationGroup);
  QString translationLanguage = settings.value (settings_names::translationLanguage,
                                                settings_values::translationLanguage).toString ();
  ui->translateLangCombo->setCurrentText (translationLanguage);
  settings.endGroup ();
}

void SettingsEditor::saveState() const
{
  QSettings settings;
  settings.beginGroup (settings_names::guiGroup);
  settings.setValue (objectName () + "_" + settings_names::geometry, saveGeometry ());
}

void SettingsEditor::loadState()
{
  QSettings settings;
  settings.beginGroup (settings_names::guiGroup);
  restoreGeometry (settings.value (objectName () + "_" + settings_names::geometry).toByteArray ());
}

void SettingsEditor::initOcrLangCombo()
{
  ui->ocrLangCombo->clear ();
  QString tessdataDir = ui->tessdataEdit->text ();
  QDir dir (tessdataDir + "tessdata/");
  if (!dir.exists ())
  {
    return;
  }
  QStringList files = dir.entryList (QStringList () << "*.traineddata", QDir::Files);
  QStringList languages;
  foreach (const QString& file, files)
  {
    QString lang = file.left (file.indexOf ("."));
    languages << lang;
  }
  ui->ocrLangCombo->addItems (languages);
}

void SettingsEditor::initTranslateLangCombo()
{
  QHash<QString, QString> gtLang;
  gtLang.insert("Afrikaans","af");
  gtLang.insert("Albanian","sq");
  gtLang.insert("Arabic","ar");
  gtLang.insert("Armenian","hy");
  gtLang.insert("Azerbaijani","az");
  gtLang.insert("Basque","eu");
  gtLang.insert("Belarusian","be");
  gtLang.insert("Bulgarian","bg");
  gtLang.insert("Catalan","ca");
  gtLang.insert("Chinese (Simplified)","zh-CN");
  gtLang.insert("Chinese (Traditional)","zh-TW");
  gtLang.insert("Croatian","hr");
  gtLang.insert("Czech","cs");
  gtLang.insert("Danish","da");
  gtLang.insert("Dutch","nl");
  gtLang.insert("English","en");
  gtLang.insert("Estonian","et");
  gtLang.insert("Filipino","tl");
  gtLang.insert("Finnish","fi");
  gtLang.insert("French","fr");
  gtLang.insert("Galician","gl");
  gtLang.insert("Georgian","ka");
  gtLang.insert("German","de");
  gtLang.insert("Greek","el");
  gtLang.insert("Haitian Creole","ht");
  gtLang.insert("Hebrew","iw");
  gtLang.insert("Hindi","hi");
  gtLang.insert("Hungarian","hu");
  gtLang.insert("Icelandic","is");
  gtLang.insert("Indonesian","id");
  gtLang.insert("Irish","ga");
  gtLang.insert("Italian","it");
  gtLang.insert("Japanese","ja");
  gtLang.insert("Korean","ko");
  gtLang.insert("Latvian","lv");
  gtLang.insert("Lithuanian","lt");
  gtLang.insert("Macedonian","mk");
  gtLang.insert("Malay","ms");
  gtLang.insert("Maltese","mt");
  gtLang.insert("Norwegian","no");
  gtLang.insert("Persian","fa");
  gtLang.insert("Polish","pl");
  gtLang.insert("Portuguese","pt");
  gtLang.insert("Romanian","ro");
  gtLang.insert("Russian","ru");
  gtLang.insert("Serbian","sr");
  gtLang.insert("Slovak","sk");
  gtLang.insert("Slovenian","sl");
  gtLang.insert("Spanish","es");
  gtLang.insert("Swahili","sw");
  gtLang.insert("Swedish","sv");
  gtLang.insert("Thai","th");
  gtLang.insert("Turkish","tr");
  gtLang.insert("Ukrainian","uk");
  gtLang.insert("Urdu","ur");
  gtLang.insert("Vietnamese","vi");
  gtLang.insert("Welsh","cy");
  gtLang.insert("Yiddish","yi");
  ui->translateLangCombo->addItems (gtLang.values ());
}

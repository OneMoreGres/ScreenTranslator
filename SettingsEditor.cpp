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

  initTranslateLanguages ();
  ui->translateLangCombo->addItems (translateLanguages_.keys ());
  initOcrLanguages ();
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
  QString uiOcrLanguage = ui->ocrLangCombo->currentText ();
  QString settingsOcrLanguage = ocrLanguages_.value (uiOcrLanguage, uiOcrLanguage);
  settings.setValue (settings_names::ocrLanguage, settingsOcrLanguage);
  settings.setValue (settings_names::imageScale, ui->imageScaleSpin->value ());
  settings.endGroup ();


  settings.beginGroup (settings_names::translationGroup);
  Q_ASSERT (translateLanguages_.contains (ui->translateLangCombo->currentText ()));
  QString uiTranslateLanguage = ui->translateLangCombo->currentText ();
  QString settingsTranslateLanguage = translateLanguages_.value (uiTranslateLanguage);
  settings.setValue (settings_names::translationLanguage, settingsTranslateLanguage);
  QString sourceLanguage = settings_values::sourceLanguage;
  if (translateLanguages_.contains (uiOcrLanguage))
  {
    sourceLanguage = translateLanguages_.value (uiOcrLanguage);
  }
  settings.setValue (settings_names::sourceLanguage, sourceLanguage);
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
  QString settingsOcrLanguage = settings.value (settings_names::ocrLanguage,
                                                settings_values::ocrLanguage).toString ();
  QString uiOcrLanguage = ocrLanguages_.key (settingsOcrLanguage, settingsOcrLanguage);
  ui->ocrLangCombo->setCurrentText (uiOcrLanguage);
  int imageScale = settings.value (settings_names::imageScale,
                                   settings_values::imageScale).toInt ();
  ui->imageScaleSpin->setValue (imageScale);
  settings.endGroup ();


  settings.beginGroup (settings_names::translationGroup);
  QString settingsTranslateLanguage = settings.value (settings_names::translationLanguage,
                                                      settings_values::translationLanguage).toString ();
  QString uiTranslateLanguage = translateLanguages_.key (settingsTranslateLanguage);
  Q_ASSERT (!uiTranslateLanguage.isEmpty ());
  ui->translateLangCombo->setCurrentText (uiTranslateLanguage);
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
  QDir dir (tessdataDir + "/tessdata/");
  if (!dir.exists ())
  {
    return;
  }
  QStringList files = dir.entryList (QStringList () << "*.traineddata", QDir::Files);
  QStringList languages;
  foreach (const QString& file, files)
  {
    QString lang = file.left (file.indexOf ("."));
    lang = ocrLanguages_.key (lang, lang); // Replace with readable text if can.
    languages << lang;
  }
  ui->ocrLangCombo->addItems (languages);
}

void SettingsEditor::initTranslateLanguages()
{
  translateLanguages_.insert(tr("Afrikaans"),"af");
  translateLanguages_.insert(tr("Albanian"),"sq");
  translateLanguages_.insert(tr("Arabic"),"ar");
  translateLanguages_.insert(tr("Armenian"),"hy");
  translateLanguages_.insert(tr("Azerbaijani"),"az");
  translateLanguages_.insert(tr("Basque"),"eu");
  translateLanguages_.insert(tr("Belarusian"),"be");
  translateLanguages_.insert(tr("Bulgarian"),"bg");
  translateLanguages_.insert(tr("Catalan"),"ca");
  translateLanguages_.insert(tr("Chinese (Simplified)"),"zh-CN");
  translateLanguages_.insert(tr("Chinese (Traditional)"),"zh-TW");
  translateLanguages_.insert(tr("Croatian"),"hr");
  translateLanguages_.insert(tr("Czech"),"cs");
  translateLanguages_.insert(tr("Danish"),"da");
  translateLanguages_.insert(tr("Dutch"),"nl");
  translateLanguages_.insert(tr("English"),"en");
  translateLanguages_.insert(tr("Estonian"),"et");
  translateLanguages_.insert(tr("Filipino"),"tl");
  translateLanguages_.insert(tr("Finnish"),"fi");
  translateLanguages_.insert(tr("French"),"fr");
  translateLanguages_.insert(tr("Galician"),"gl");
  translateLanguages_.insert(tr("Georgian"),"ka");
  translateLanguages_.insert(tr("German"),"de");
  translateLanguages_.insert(tr("Greek"),"el");
  translateLanguages_.insert(tr("Haitian Creole"),"ht");
  translateLanguages_.insert(tr("Hebrew"),"iw");
  translateLanguages_.insert(tr("Hindi"),"hi");
  translateLanguages_.insert(tr("Hungarian"),"hu");
  translateLanguages_.insert(tr("Icelandic"),"is");
  translateLanguages_.insert(tr("Indonesian"),"id");
  translateLanguages_.insert(tr("Irish"),"ga");
  translateLanguages_.insert(tr("Italian"),"it");
  translateLanguages_.insert(tr("Japanese"),"ja");
  translateLanguages_.insert(tr("Korean"),"ko");
  translateLanguages_.insert(tr("Latvian"),"lv");
  translateLanguages_.insert(tr("Lithuanian"),"lt");
  translateLanguages_.insert(tr("Macedonian"),"mk");
  translateLanguages_.insert(tr("Malay"),"ms");
  translateLanguages_.insert(tr("Maltese"),"mt");
  translateLanguages_.insert(tr("Norwegian"),"no");
  translateLanguages_.insert(tr("Persian"),"fa");
  translateLanguages_.insert(tr("Polish"),"pl");
  translateLanguages_.insert(tr("Portuguese"),"pt");
  translateLanguages_.insert(tr("Romanian"),"ro");
  translateLanguages_.insert(tr("Russian"),"ru");
  translateLanguages_.insert(tr("Serbian"),"sr");
  translateLanguages_.insert(tr("Slovak"),"sk");
  translateLanguages_.insert(tr("Slovenian"),"sl");
  translateLanguages_.insert(tr("Spanish"),"es");
  translateLanguages_.insert(tr("Swahili"),"sw");
  translateLanguages_.insert(tr("Swedish"),"sv");
  translateLanguages_.insert(tr("Thai"),"th");
  translateLanguages_.insert(tr("Turkish"),"tr");
  translateLanguages_.insert(tr("Ukrainian"),"uk");
  translateLanguages_.insert(tr("Urdu"),"ur");
  translateLanguages_.insert(tr("Vietnamese"),"vi");
  translateLanguages_.insert(tr("Welsh"),"cy");
  translateLanguages_.insert(tr("Yiddish"),"yi");
}

void SettingsEditor::initOcrLanguages()
{
  ocrLanguages_.insert(tr("Ancient Greek"),"grc");
  ocrLanguages_.insert(tr("Esperanto alternative"),"epo_alt");
  ocrLanguages_.insert(tr("English"),"eng");
  ocrLanguages_.insert(tr("Ukrainian"),"ukr");
  ocrLanguages_.insert(tr("Turkish"),"tur");
  ocrLanguages_.insert(tr("Thai"),"tha");
  ocrLanguages_.insert(tr("Tagalog"),"tgl");
  ocrLanguages_.insert(tr("Telugu"),"tel");
  ocrLanguages_.insert(tr("Tamil"),"tam");
  ocrLanguages_.insert(tr("Swedish"),"swe");
  ocrLanguages_.insert(tr("Swahili"),"swa");
  ocrLanguages_.insert(tr("Serbian"),"srp");
  ocrLanguages_.insert(tr("Albanian"),"sqi");
  ocrLanguages_.insert(tr("Spanish"),"spa");
  ocrLanguages_.insert(tr("Slovenian"),"slv");
  ocrLanguages_.insert(tr("Slovakian"),"slk");
  ocrLanguages_.insert(tr("Romanian"),"ron");
  ocrLanguages_.insert(tr("Portuguese"),"por");
  ocrLanguages_.insert(tr("Polish"),"pol");
  ocrLanguages_.insert(tr("Norwegian"),"nor");
  ocrLanguages_.insert(tr("Dutch"),"nld");
  ocrLanguages_.insert(tr("Malay"),"msa");
  ocrLanguages_.insert(tr("Maltese"),"mlt");
  ocrLanguages_.insert(tr("Macedonian"),"mkd");
  ocrLanguages_.insert(tr("Malayalam"),"mal");
  ocrLanguages_.insert(tr("Lithuanian"),"lit");
  ocrLanguages_.insert(tr("Latvian"),"lav");
  ocrLanguages_.insert(tr("Korean"),"kor");
  ocrLanguages_.insert(tr("Kannada"),"kan");
  ocrLanguages_.insert(tr("Italian"),"ita");
  ocrLanguages_.insert(tr("Icelandic"),"isl");
  ocrLanguages_.insert(tr("Indonesian"),"ind");
  ocrLanguages_.insert(tr("Cherokee"),"chr");
  ocrLanguages_.insert(tr("Hungarian"),"hun");
  ocrLanguages_.insert(tr("Croatian"),"hrv");
  ocrLanguages_.insert(tr("Hindi"),"hin");
  ocrLanguages_.insert(tr("Hebrew"),"heb");
  ocrLanguages_.insert(tr("Galician"),"glg");
  ocrLanguages_.insert(tr("Middle French (ca. 1400-1600)"),"frm");
  ocrLanguages_.insert(tr("Frankish"),"frk");
  ocrLanguages_.insert(tr("French"),"fra");
  ocrLanguages_.insert(tr("Finnish"),"fin");
  ocrLanguages_.insert(tr("Basque"),"eus");
  ocrLanguages_.insert(tr("Estonian"),"est");
  ocrLanguages_.insert(tr("Math / equation"),"equ");
  ocrLanguages_.insert(tr("Esperanto"),"epo");
  ocrLanguages_.insert(tr("Middle English (1100-1500)"),"enm");
  ocrLanguages_.insert(tr("Greek"),"ell");
  ocrLanguages_.insert(tr("German"),"deu");
  ocrLanguages_.insert(tr("Danish"),"dan");
  ocrLanguages_.insert(tr("Czech"),"ces");
  ocrLanguages_.insert(tr("Catalan"),"cat");
  ocrLanguages_.insert(tr("Bulgarian"),"bul");
  ocrLanguages_.insert(tr("Bengali"),"ben");
  ocrLanguages_.insert(tr("Belarusian"),"bel");
  ocrLanguages_.insert(tr("Azerbaijani"),"aze");
  ocrLanguages_.insert(tr("Arabic"),"ara");
  ocrLanguages_.insert(tr("Afrikaans"),"afr");
  ocrLanguages_.insert(tr("Japanese"),"jpn");
  ocrLanguages_.insert(tr("Chinese (Simplified)"),"chi_sim");
  ocrLanguages_.insert(tr("Chinese (Traditional)"),"chi_tra");
  ocrLanguages_.insert(tr("Russian"),"rus");
  ocrLanguages_.insert(tr("Vietnamese"),"vie");
}

#include "settingseditor.h"
#include "languagecodes.h"
#include "ui_settingseditor.h"
#include "widgetstate.h"

#include <QFileDialog>
#include <QNetworkProxy>
#include <QStringListModel>

SettingsEditor::SettingsEditor()
  : ui(new Ui::SettingsEditor)
{
  ui->setupUi(this);

  {
    auto model = new QStringListModel(this);
    model->setStringList({tr("General"), tr("Recognition"), tr("Correction"),
                          tr("Translation"), tr("Representation"),
                          tr("Update")});
    ui->pagesList->setModel(model);
    auto selection = ui->pagesList->selectionModel();
    connect(selection, &QItemSelectionModel::currentRowChanged,  //
            this, &SettingsEditor::updateCurrentPage);
  }

  // general
  //  QMap<QNetworkProxy::ProxyType, QString> proxyTypeNames;
  //  proxyTypeNames.insert(QNetworkProxy::NoProxy, tr("No"));
  //  proxyTypeNames.insert(QNetworkProxy::DefaultProxy, tr("System"));
  //  proxyTypeNames.insert(QNetworkProxy::Socks5Proxy, tr("SOCKS 5"));
  //  proxyTypeNames.insert(QNetworkProxy::HttpProxy, tr("HTTP"));
  //  QList<int> proxyOrder = proxyTypeOrder();
  //  for (int type : proxyOrder) {
  //    ui->proxyTypeCombo->addItem(
  //        proxyTypeNames.value(QNetworkProxy::ProxyType(type)));
  //  }

  //  QRegExp urlRegexp(
  //      R"(^(https?:\/\/)?([\da-z\.-]+)\.([a-z\.]{2,6})([\/\w \.-]*)*\/?$)");
  //  ui->proxyHostEdit->setValidator(
  //      new QRegExpValidator(urlRegexp, ui->proxyHostEdit));
  //  ui->proxyPassEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);

  // recognition
  connect(ui->tessdataButton, &QPushButton::clicked,  //
          this, &SettingsEditor::openTessdataDialog);
  connect(ui->tessdataEdit, &QLineEdit::textChanged,  //
          this, &SettingsEditor::updateTesseractLanguages);

  //  connect(ui->recognizerFixTable, SIGNAL(itemChanged(QTableWidgetItem *)),
  //          SLOT(recognizerFixTableItemChanged(QTableWidgetItem *)));

  //  //  ui->translateLangCombo->addItems(dictionary_.translateLanguagesUi());

  // translation

  updateTranslationLanguages();

  // updates
  ui->updateCombo->addItems(
      {tr("Never"), tr("Daily"), tr("Weekly"), tr("Monthly")});

  new WidgetState(this);
}

SettingsEditor::~SettingsEditor()
{
  delete ui;
}

Settings SettingsEditor::settings() const
{
  Settings settings;
  settings.captureHotkey = ui->captureEdit->keySequence().toString();
  settings.repeatCaptureHotkey =
      ui->repeatCaptureEdit->keySequence().toString();
  settings.showLastHotkey = ui->repeatEdit->keySequence().toString();
  settings.clipboardHotkey = ui->clipboardEdit->keySequence().toString();

  settings.tessdataPath = ui->tessdataEdit->text();

  settings.doTranslation = ui->doTranslationCheck->isChecked();
  settings.ignoreSslErrors = ui->ignoreSslCheck->isChecked();
  settings.debugMode = ui->translatorDebugCheck->isChecked();
  settings.translationTimeout =
      std::chrono::seconds(ui->translateTimeoutSpin->value());

  settings.translators.clear();
  for (auto i = 0, end = ui->translatorList->count(); i < end; ++i) {
    auto item = ui->translatorList->item(i);
    if (item->checkState() == Qt::Checked)
      settings.translators.append(item->text());
  }

  settings.resultShowType =
      ui->trayRadio->isChecked() ? ResultMode::Tooltip : ResultMode::Widget;
  return settings;
}

void SettingsEditor::setSettings(const Settings &settings)
{
  ui->captureEdit->setKeySequence(settings.captureHotkey);
  ui->repeatCaptureEdit->setKeySequence(settings.repeatCaptureHotkey);
  ui->repeatEdit->setKeySequence(settings.showLastHotkey);
  ui->clipboardEdit->setKeySequence(settings.clipboardHotkey);

  ui->tessdataEdit->setText(settings.tessdataPath);
  updateTesseractLanguages();

  ui->doTranslationCheck->setChecked(settings.doTranslation);
  ui->ignoreSslCheck->setChecked(settings.ignoreSslErrors);
  ui->translatorDebugCheck->setChecked(settings.debugMode);
  ui->translateTimeoutSpin->setValue(settings.translationTimeout.count());
  translatorsDir_ = settings.translatorsDir;
  updateTranslators(settings.translators);

  ui->trayRadio->setChecked(settings.resultShowType == ResultMode::Tooltip);
  ui->dialogRadio->setChecked(settings.resultShowType == ResultMode::Widget);
}

void SettingsEditor::updateCurrentPage()
{
  ui->pagesView->setCurrentIndex(ui->pagesList->currentIndex().row());
}

void SettingsEditor::openTessdataDialog()
{
  const auto path =
      QFileDialog::getExistingDirectory(this, tr("Path to tessdata"));

  if (path.isEmpty())
    return;

  ui->tessdataEdit->setText(path);
}

void SettingsEditor::updateTesseractLanguages()
{
  ui->tesseractLangCombo->clear();
  ui->correctLangCombo->clear();

  QDir dir(ui->tessdataEdit->text());
  if (!dir.exists())
    return;

  LanguageIds names;
  LanguageCodes languages;

  const auto files = dir.entryList({"*.traineddata"}, QDir::Files);
  for (const auto &file : files) {
    const auto lang = file.left(file.indexOf("."));
    if (const auto bundle = languages.findByTesseract(lang))
      names.append(bundle->name);
  }

  if (names.isEmpty())
    return;

  std::sort(names.begin(), names.end());
  ui->tesseractLangCombo->addItems(names);
  ui->correctLangCombo->addItems(names);
}

void SettingsEditor::updateCorrectionsTable()
{
}

void SettingsEditor::updateTranslators(const QStringList &enabled)
{
  ui->translatorList->clear();

  QDir dir(translatorsDir_);
  if (!dir.exists())
    return;

  auto files = dir.entryList({"*.js"}, QDir::Files);
  std::sort(files.begin(), files.end());
  ui->translatorList->addItems(files);

  for (auto i = 0, end = ui->translatorList->count(); i < end; ++i) {
    auto item = ui->translatorList->item(i);
    item->setCheckState(enabled.contains(item->text()) ? Qt::Checked
                                                       : Qt::Unchecked);
  }
}

void SettingsEditor::updateTranslationLanguages()
{
  LanguageIds names;
  LanguageCodes languages;

  for (const auto &bundle : languages.all()) {
    if (!bundle.second.iso639_1.isEmpty())
      names.append(bundle.second.name);
  }

  ui->translateLangCombo->clear();
  std::sort(names.begin(), names.end());
  ui->translateLangCombo->addItems(names);
}

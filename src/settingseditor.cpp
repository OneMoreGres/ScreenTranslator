#include "settingseditor.h"
#include "languagecodes.h"
#include "ui_settingseditor.h"
#include "updates.h"
#include "widgetstate.h"

#include <QFileDialog>
#include <QNetworkProxy>
#include <QSortFilterProxyModel>
#include <QStringListModel>

SettingsEditor::SettingsEditor(update::Loader &updater)
  : ui(new Ui::SettingsEditor)
  , updater_(updater)
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

  {
    QMap<ProxyType, QString> proxyTypes;
    proxyTypes.insert(ProxyType::Disabled, tr("Disabled"));
    proxyTypes.insert(ProxyType::System, tr("System"));
    proxyTypes.insert(ProxyType::Socks5, tr("SOCKS 5"));
    proxyTypes.insert(ProxyType::Http, tr("HTTP"));
    ui->proxyTypeCombo->addItems(proxyTypes.values());

    QRegExp urlRegexp(
        R"(^(https?:\/\/)?([\da-z\.-]+)\.([a-z\.]{2,6})([\/\w \.-]*)*\/?$)");
    ui->proxyHostEdit->setValidator(
        new QRegExpValidator(urlRegexp, ui->proxyHostEdit));

    ui->proxyPassEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
  }

  // correction

  ui->userSubstitutionsTable->setEnabled(ui->useUserSubstitutions->isChecked());
  connect(ui->useUserSubstitutions, &QCheckBox::toggled,  //
          ui->userSubstitutionsTable, &QTableWidget::setEnabled);

  // translation
  updateTranslationLanguages();

  // updates
  QMap<AutoUpdate, QString> updateTypes;
  updateTypes.insert(AutoUpdate::Disabled, tr("Disabled"));
  updateTypes.insert(AutoUpdate::Daily, tr("Daily"));
  updateTypes.insert(AutoUpdate::Weekly, tr("Weekly"));
  updateTypes.insert(AutoUpdate::Monthly, tr("Monthly"));
  ui->updateCombo->addItems(updateTypes.values());

  auto updatesProxy = new QSortFilterProxyModel(this);
  updatesProxy->setSourceModel(updater_.model());
  ui->updatesView->setModel(updatesProxy);
  ui->updatesView->setItemDelegateForColumn(int(update::Model::Column::Action),
                                            new update::ActionDelegate(this));
#ifndef DEVELOP
  ui->updatesView->hideColumn(int(update::Model::Column::Files));
#endif
  adjustUpdatesView();
  connect(updater_.model(), &QAbstractItemModel::modelReset,  //
          this, &SettingsEditor::adjustUpdatesView);
  connect(&updater_, &update::Loader::updated,  //
          this, &SettingsEditor::adjustUpdatesView);
  connect(ui->checkUpdates, &QPushButton::clicked,  //
          &updater_, &update::Loader::checkForUpdates);
  connect(ui->applyUpdates, &QPushButton::clicked,  //
          &updater_, &update::Loader::applyUserActions);

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

  settings.showMessageOnStart = ui->showOnStart->isChecked();

  settings.proxyType = ProxyType(ui->proxyTypeCombo->currentIndex());
  settings.proxyHostName = ui->proxyHostEdit->text();
  settings.proxyPort = ui->proxyPortSpin->value();
  settings.proxyUser = ui->proxyUserEdit->text();
  settings.proxyPassword = ui->proxyPassEdit->text();
  settings.proxySavePassword = ui->proxySaveCheck->isChecked();

  LanguageCodes langs;
  if (auto lang = langs.findByName(ui->tesseractLangCombo->currentText()))
    settings.sourceLanguage = lang->id;

  settings.useUserSubstitutions = ui->useUserSubstitutions->isChecked();
  settings.userSubstitutions = ui->userSubstitutionsTable->substitutions();

  settings.doTranslation = ui->doTranslationCheck->isChecked();
  settings.ignoreSslErrors = ui->ignoreSslCheck->isChecked();
  settings.debugMode = ui->translatorDebugCheck->isChecked();
  settings.translationTimeout =
      std::chrono::seconds(ui->translateTimeoutSpin->value());
  if (auto lang = langs.findByName(ui->translateLangCombo->currentText()))
    settings.targetLanguage = lang->id;

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

  ui->showOnStart->setChecked(settings.showMessageOnStart);

  ui->proxyTypeCombo->setCurrentIndex(int(settings.proxyType));
  ui->proxyHostEdit->setText(settings.proxyHostName);
  ui->proxyPortSpin->setValue(settings.proxyPort);
  ui->proxyUserEdit->setText(settings.proxyUser);
  ui->proxyPassEdit->setText(settings.proxyPassword);
  ui->proxySaveCheck->setChecked(settings.proxySavePassword);

  LanguageCodes langs;
  ui->tessdataPath->setText(settings.tessdataPath);
  updateTesseractLanguages();
  if (auto lang = langs.findById(settings.sourceLanguage))
    ui->tesseractLangCombo->setCurrentText(lang->name);

  ui->useUserSubstitutions->setChecked(settings.useUserSubstitutions);
  ui->userSubstitutionsTable->setSubstitutions(settings.userSubstitutions);

  ui->doTranslationCheck->setChecked(settings.doTranslation);
  ui->ignoreSslCheck->setChecked(settings.ignoreSslErrors);
  ui->translatorDebugCheck->setChecked(settings.debugMode);
  ui->translateTimeoutSpin->setValue(settings.translationTimeout.count());
  ui->translatorsPath->setText(settings.translatorsDir);
  enabledTranslators_ = settings.translators;
  updateTranslators();
  if (auto lang = langs.findById(settings.targetLanguage))
    ui->translateLangCombo->setCurrentText(lang->name);

  ui->trayRadio->setChecked(settings.resultShowType == ResultMode::Tooltip);
  ui->dialogRadio->setChecked(settings.resultShowType == ResultMode::Widget);
}

void SettingsEditor::updateCurrentPage()
{
  ui->pagesView->setCurrentIndex(ui->pagesList->currentIndex().row());
}

void SettingsEditor::updateTesseractLanguages()
{
  ui->tesseractLangCombo->clear();

  const auto path = ui->tessdataPath->text();
  if (path.isEmpty())
    return;

  QDir dir(path);
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
}

void SettingsEditor::updateTranslators()
{
  ui->translatorList->clear();

  const auto path = ui->translatorsPath->text();
  if (path.isEmpty())
    return;

  QDir dir(path);
  if (!dir.exists())
    return;

  auto files = dir.entryList({"*.js"}, QDir::Files);
  std::sort(files.begin(), files.end());
  ui->translatorList->addItems(files);

  for (auto i = 0, end = ui->translatorList->count(); i < end; ++i) {
    auto item = ui->translatorList->item(i);
    item->setCheckState(enabledTranslators_.contains(item->text())
                            ? Qt::Checked
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

void SettingsEditor::adjustUpdatesView()
{
  ui->updatesView->resizeColumnToContents(int(update::Model::Column::Name));
  updateTesseractLanguages();
  updateTranslators();
}

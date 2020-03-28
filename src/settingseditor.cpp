#include "settingseditor.h"
#include "languagecodes.h"
#include "manager.h"
#include "runatsystemstart.h"
#include "tesseract.h"
#include "translator.h"
#include "ui_settingseditor.h"
#include "updates.h"
#include "widgetstate.h"

#include <QFileDialog>
#include <QSortFilterProxyModel>
#include <QStringListModel>

SettingsEditor::SettingsEditor(Manager &manager, update::Loader &updater)
  : ui(new Ui::SettingsEditor)
  , manager_(manager)
  , updater_(updater)
{
  ui->setupUi(this);

  connect(ui->buttonBox, &QDialogButtonBox::clicked,  //
          this, &SettingsEditor::handleButtonBoxClicked);

  connect(ui->portable, &QCheckBox::toggled,  //
          this, &SettingsEditor::handlePortableChanged);

  ui->runAtSystemStart->setEnabled(service::RunAtSystemStart::isAvailable());

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

  // translation
  ui->tesseractLangCombo->setModel(models_.sourceLanguageModel());

  // correction
  ui->userSubstitutionsTable->setEnabled(ui->useUserSubstitutions->isChecked());
  ui->userSubstitutionsTable->setSourceLanguageModel(
      models_.sourceLanguageModel());
  connect(ui->useUserSubstitutions, &QCheckBox::toggled,  //
          ui->userSubstitutionsTable, &QTableWidget::setEnabled);

  // translation
  ui->translateLangCombo->setModel(models_.targetLanguageModel());

  // representation
  connect(ui->dialogRadio, &QRadioButton::toggled,  //
          ui->resultWindow, &QTableWidget::setEnabled);
  connect(ui->resultFont, &QFontComboBox::currentFontChanged,  //
          this, &SettingsEditor::updateResultFont);
  connect(ui->resultFontSize, qOverload<int>(&QSpinBox::valueChanged),  //
          this, &SettingsEditor::updateResultFont);

  // updates
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

  new service::WidgetState(this);
}

SettingsEditor::~SettingsEditor()
{
  delete ui;
}

Settings SettingsEditor::settings() const
{
  Settings settings;
  settings.setPortable(ui->portable->isChecked());

  settings.runAtSystemStart = ui->runAtSystemStart->isChecked();

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

  settings.sourceLanguage =
      LanguageCodes::idForName(ui->tesseractLangCombo->currentText());

  settings.useUserSubstitutions = ui->useUserSubstitutions->isChecked();
  settings.userSubstitutions = ui->userSubstitutionsTable->substitutions();

  settings.doTranslation = ui->doTranslationCheck->isChecked();
  settings.ignoreSslErrors = ui->ignoreSslCheck->isChecked();
  settings.debugMode = ui->translatorDebugCheck->isChecked();
  settings.translationTimeout =
      std::chrono::seconds(ui->translateTimeoutSpin->value());
  settings.targetLanguage =
      LanguageCodes::idForName(ui->translateLangCombo->currentText());

  settings.translators.clear();
  for (auto i = 0, end = ui->translatorList->count(); i < end; ++i) {
    auto item = ui->translatorList->item(i);
    if (item->checkState() == Qt::Checked)
      settings.translators.append(item->text());
  }

  settings.resultShowType =
      ui->trayRadio->isChecked() ? ResultMode::Tooltip : ResultMode::Widget;
  settings.fontFamily = ui->resultFont->currentFont().family();
  settings.fontSize = ui->resultFontSize->value();
  settings.showRecognized = ui->showRecognized->isChecked();
  settings.showCaptured = ui->showCaptured->isChecked();

  settings.autoUpdateIntervalDays = ui->autoUpdateInterval->value();

  return settings;
}

void SettingsEditor::setSettings(const Settings &settings)
{
  if (settings.isPortable() == ui->portable->isChecked())
    updateModels(settings.tessdataPath);

  wasPortable_ = settings.isPortable();
  ui->portable->setChecked(settings.isPortable());

  ui->runAtSystemStart->setChecked(settings.runAtSystemStart);

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

  ui->tessdataPath->setText(settings.tessdataPath);
  ui->tesseractLangCombo->setCurrentText(
      LanguageCodes::name(settings.sourceLanguage));

  ui->useUserSubstitutions->setChecked(settings.useUserSubstitutions);
  ui->userSubstitutionsTable->setSubstitutions(settings.userSubstitutions);

  ui->doTranslationCheck->setChecked(settings.doTranslation);
  ui->ignoreSslCheck->setChecked(settings.ignoreSslErrors);
  ui->translatorDebugCheck->setChecked(settings.debugMode);
  ui->translateTimeoutSpin->setValue(settings.translationTimeout.count());
  ui->translatorsPath->setText(settings.translatorsDir);
  enabledTranslators_ = settings.translators;
  updateTranslators();
  ui->translateLangCombo->setCurrentText(
      LanguageCodes::name(settings.targetLanguage));

  ui->trayRadio->setChecked(settings.resultShowType == ResultMode::Tooltip);
  ui->dialogRadio->setChecked(settings.resultShowType == ResultMode::Widget);
  ui->resultFont->setCurrentFont(QFont(settings.fontFamily));
  ui->resultFontSize->setValue(settings.fontSize);
  ui->showRecognized->setChecked(settings.showRecognized);
  ui->showCaptured->setChecked(settings.showCaptured);

  ui->autoUpdateInterval->setValue(settings.autoUpdateIntervalDays);
}

void SettingsEditor::updateCurrentPage()
{
  ui->pagesView->setCurrentIndex(ui->pagesList->currentIndex().row());
}

void SettingsEditor::updateTranslators()
{
  ui->translatorList->clear();

  auto names = Translator::availableTranslators(ui->translatorsPath->text());
  if (names.isEmpty())
    return;

  std::sort(names.begin(), names.end());
  ui->translatorList->addItems(names);

  for (auto i = 0, end = ui->translatorList->count(); i < end; ++i) {
    auto item = ui->translatorList->item(i);
    item->setCheckState(enabledTranslators_.contains(item->text())
                            ? Qt::Checked
                            : Qt::Unchecked);
  }
}

void SettingsEditor::adjustUpdatesView()
{
  ui->updatesView->resizeColumnToContents(int(update::Model::Column::Name));

  if (ui->tessdataPath->text().isEmpty())  // not inited yet
    return;

  updateModels(ui->tessdataPath->text());
  updateTranslators();
}

void SettingsEditor::handleButtonBoxClicked(QAbstractButton *button)
{
  if (!button)
    return;

  if (button == ui->buttonBox->button(QDialogButtonBox::Ok)) {
    accept();
    return;
  }
  if (button == ui->buttonBox->button(QDialogButtonBox::Cancel)) {
    reject();
    return;
  }
  if (button == ui->buttonBox->button(QDialogButtonBox::Apply)) {
    const auto settings = this->settings();
    manager_.applySettings(settings);
    if (settings.isPortable() != wasPortable_) {
      wasPortable_ = settings.isPortable();
      handlePortableChanged();
    }
    return;
  }
}

void SettingsEditor::handlePortableChanged()
{
  Settings settings;
  settings.setPortable(ui->portable->isChecked());
  ui->tessdataPath->setText(settings.tessdataPath);
  ui->translatorsPath->setText(settings.translatorsDir);
  updateModels(settings.tessdataPath);
  updateTranslators();

  const auto portableChanged = wasPortable_ != settings.isPortable();
  ui->pageUpdate->setEnabled(!portableChanged);
  ui->pageUpdate->setToolTip(portableChanged
                                 ? tr("Portable changed. Apply settings first")
                                 : QString());
}

void SettingsEditor::updateResultFont()
{
  auto font = ui->resultFont->currentFont();
  font.setPointSize(ui->resultFontSize->value());
  ui->resultFont->setFont(font);
}

void SettingsEditor::updateModels(const QString &tessdataPath)
{
  const auto source = ui->tesseractLangCombo->currentText();
  models_.update(tessdataPath);
  ui->tesseractLangCombo->setCurrentText(source);
}

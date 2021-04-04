#include "settingseditor.h"
#include "languagecodes.h"
#include "manager.h"
#include "runatsystemstart.h"
#include "translator.h"
#include "ui_settingseditor.h"
#include "updates.h"
#include "widgetstate.h"

#include <QColorDialog>

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
                          tr("Translation"), tr("Representation"), tr("Update"),
                          tr("Help")});
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

  // recognition
  ui->tesseractLangCombo->setModel(models_.sourceLanguageModel());
  const QMap<TesseractVersion, QString> tesseractVersions{
      {TesseractVersion::Optimized, tr("Optimized")},
      {TesseractVersion::Compatible, tr("Compatible")},
  };
  ui->tesseractVersion->addItems(tesseractVersions.values());
  ui->tesseractVersion->setToolTip(
      tr("Use compatible version if you are experiencing crashes during "
         "recognition"));

  // correction
  ui->userSubstitutionsTable->setEnabled(ui->useUserSubstitutions->isChecked());
  ui->userSubstitutionsTable->setSourceLanguageModel(
      models_.sourceLanguageModel());
  connect(ui->useUserSubstitutions, &QCheckBox::toggled,  //
          ui->userSubstitutionsTable, &QTableWidget::setEnabled);

  // translation
  ui->translatorHint->setText(
      tr("<b>NOTE! Some translators might require the translation window to be "
         "visible. You can make it using the \"Show translator\" entry "
         "in the tray icon's context menu</b>"));
  ui->translateLangCombo->setModel(models_.targetLanguageModel());

  // representation
  ui->fontColor->setAutoFillBackground(true);
  ui->backgroundColor->setAutoFillBackground(true);
  ui->backgroundColor->setText(tr("Sample text"));
  connect(ui->dialogRadio, &QRadioButton::toggled,  //
          ui->resultWindow, &QTableWidget::setEnabled);
  connect(ui->resultFont, &QFontComboBox::currentFontChanged,  //
          this, &SettingsEditor::updateResultFont);
  connect(ui->resultFontSize, qOverload<int>(&QSpinBox::valueChanged),  //
          this, &SettingsEditor::updateResultFont);
  connect(ui->fontColor, &QPushButton::clicked,  //
          this, [this] { pickColor(ColorContext::Font); });
  connect(ui->backgroundColor, &QPushButton::clicked,  //
          this, [this] { pickColor(ColorContext::Bagkround); });

  // updates
  updater.model()->initView(ui->updatesView);
  adjustUpdatesView();
  connect(updater_.model(), &QAbstractItemModel::modelReset,  //
          this, &SettingsEditor::adjustUpdatesView);
  connect(&updater_, &update::Loader::updated,  //
          this, &SettingsEditor::adjustUpdatesView);
  connect(ui->checkUpdates, &QPushButton::clicked,  //
          &updater_, &update::Loader::checkForUpdates);
  connect(ui->applyUpdates, &QPushButton::clicked,  //
          &updater_, &update::Loader::applyUserActions);

  // about
  {
    const auto mail = "translator@gres.biz";
    const QString baseUrl = "https://github.com/OneMoreGres/ScreenTranslator";
    const auto issues = baseUrl + "/issues";
    QLocale locale;
    const auto changelog =
        baseUrl + "/blob/master/share/Changelog_" +
        (locale.language() == QLocale::Russian ? "ru" : "en") + ".md";
    const auto license = baseUrl + "/blob/master/LICENSE.md";
    const auto aboutLines = QStringList{
        QObject::tr(
            R"(<p>Optical character recognition (OCR) and translation tool</p>)"),
        QObject::tr(R"(<p>Version: %1</p>)")
            .arg(QApplication::applicationVersion()),
        QObject::tr(R"(<p>Changelog: <a href="%1">%2</a></p>)")
            .arg(changelog, QUrl(changelog).fileName()),
        QObject::tr(R"(<p>License: <a href="%3">MIT</a></p>)").arg(license),
        QObject::tr(R"(<p>Author: Gres (<a href="mailto:%1">%1</a>)</p>)")
            .arg(mail),
        QObject::tr(R"(<p>Issues: <a href="%1">%1</a></p>)").arg(issues),
    };

    ui->aboutLabel->setText(aboutLines.join('\n'));
    ui->aboutLabel->setTextFormat(Qt::RichText);
    ui->aboutLabel->setOpenExternalLinks(true);

    ui->helpLabel->setText(
        tr("The program workflow consists of the following steps:\n"
           "1. Selection on the screen area\n"
           "2. Recognition of the selected area\n"
           "3. Correction of the recognized text (optional)\n"
           "4. Translation of the corrected text (optional)\n"
           "User interaction is only required for step 1.\n"
           "Steps 2, 3 and 4 require additional data that can be "
           "downloaded from "
           "the updates page.\n"
           "\n"
           "At first start, go to the updates page and install desired "
           "recognition languages and translators and, optionally, "
           "hunspell "
           "dictionaries.\n"
           "Then set default recognition and translation languages, "
           "enable some "
           "(or all) translators and the \"translate text\" setting, "
           "if needed."));
  }

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
  settings.captureLockedHotkey =
      ui->captureLockedEdit->keySequence().toString();

  settings.showMessageOnStart = ui->showOnStart->isChecked();
  settings.writeTrace = ui->writeTrace->isChecked();

  settings.proxyType = ProxyType(ui->proxyTypeCombo->currentIndex());
  settings.proxyHostName = ui->proxyHostEdit->text();
  settings.proxyPort = ui->proxyPortSpin->value();
  settings.proxyUser = ui->proxyUserEdit->text();
  settings.proxyPassword = ui->proxyPassEdit->text();
  settings.proxySavePassword = ui->proxySaveCheck->isChecked();

  settings.sourceLanguage =
      LanguageCodes::idForName(ui->tesseractLangCombo->currentText());
  settings.tesseractVersion =
      TesseractVersion(ui->tesseractVersion->currentIndex());

  settings.useHunspell = ui->useHunspell->isChecked();
  settings.useUserSubstitutions = ui->useUserSubstitutions->isChecked();
  settings.userSubstitutions = ui->userSubstitutionsTable->substitutions();

  settings.doTranslation = ui->doTranslationCheck->isChecked();
  settings.ignoreSslErrors = ui->ignoreSslCheck->isChecked();
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
  settings.fontColor = ui->fontColor->palette().color(QPalette::Button);
  settings.backgroundColor =
      ui->backgroundColor->palette().color(QPalette::Button);
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
  ui->captureLockedEdit->setKeySequence(settings.captureLockedHotkey);

  ui->showOnStart->setChecked(settings.showMessageOnStart);
  ui->writeTrace->setChecked(settings.writeTrace);

  ui->proxyTypeCombo->setCurrentIndex(int(settings.proxyType));
  ui->proxyHostEdit->setText(settings.proxyHostName);
  ui->proxyPortSpin->setValue(settings.proxyPort);
  ui->proxyUserEdit->setText(settings.proxyUser);
  ui->proxyPassEdit->setText(settings.proxyPassword);
  ui->proxySaveCheck->setChecked(settings.proxySavePassword);

  ui->tessdataPath->setText(settings.tessdataPath);
  ui->tesseractLangCombo->setCurrentText(
      LanguageCodes::name(settings.sourceLanguage));
  ui->tesseractVersion->setCurrentIndex(int(settings.tesseractVersion));

  ui->useHunspell->setChecked(settings.useHunspell);
  ui->hunspellDir->setText(settings.hunspellDir);
  ui->useUserSubstitutions->setChecked(settings.useUserSubstitutions);
  ui->userSubstitutionsTable->setSubstitutions(settings.userSubstitutions);

  ui->doTranslationCheck->setChecked(settings.doTranslation);
  ui->ignoreSslCheck->setChecked(settings.ignoreSslErrors);
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
  {
    QPalette palette(this->palette());
    palette.setColor(QPalette::Button, settings.fontColor);
    ui->fontColor->setPalette(palette);
    palette.setColor(QPalette::ButtonText, settings.fontColor);
    palette.setColor(QPalette::Button, settings.backgroundColor);
    ui->backgroundColor->setPalette(palette);
  }
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

  if (!enabledTranslators_.isEmpty()) {
    for (const auto &name : enabledTranslators_) names.removeOne(name);
    names = enabledTranslators_ + names;
  }

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
  ui->hunspellDir->setText(settings.hunspellDir);
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
  if (!source.isEmpty()) {
    ui->tesseractLangCombo->setCurrentText(source);
  } else if (ui->tesseractLangCombo->count() > 0) {
    ui->tesseractLangCombo->setCurrentIndex(0);
  }
}

void SettingsEditor::pickColor(ColorContext context)
{
  const auto widget =
      context == ColorContext::Font ? ui->fontColor : ui->backgroundColor;
  const auto original = widget->palette().color(QPalette::Button);
  const auto color = QColorDialog::getColor(original, this);

  if (!color.isValid())
    return;

  QPalette palette(widget->palette());
  palette.setColor(QPalette::Button, color);
  widget->setPalette(palette);

  if (context == ColorContext::Bagkround)
    return;

  palette = ui->backgroundColor->palette();
  palette.setColor(QPalette::ButtonText, color);
  ui->backgroundColor->setPalette(palette);
  ui->backgroundColor->update();
}

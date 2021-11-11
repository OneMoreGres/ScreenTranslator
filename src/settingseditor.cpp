#include "settingseditor.h"
#include "debug.h"
#include "languagecodes.h"
#include "manager.h"
#include "runatsystemstart.h"
#include "settingsvalidator.h"
#include "ui_settingseditor.h"
#include "updates.h"
#include "widgetstate.h"

#include <QColorDialog>
#include <QStandardItemModel>

namespace
{
enum class Page {  // order must match ui->pagesView
  General,
  Recognition,
  Correction,
  Translation,
  Representation,
  Update,
  Help,
  Count
};
enum class PageColumn { Name, Description, Error, Count };
}  // namespace

SettingsEditor::SettingsEditor(Manager &manager, update::Updater &updater)
  : ui(new Ui::SettingsEditor)
  , manager_(manager)
  , updater_(updater)
  , pageModel_(new QStandardItemModel(this))
{
  ui->setupUi(this);

  connect(ui->buttonBox, &QDialogButtonBox::clicked,  //
          this, &SettingsEditor::handleButtonBoxClicked);

  connect(ui->portable, &QCheckBox::toggled,  //
          this, &SettingsEditor::updateState);

  ui->runAtSystemStart->setEnabled(service::RunAtSystemStart::isAvailable());

  {
    struct Info {
      QString title;
      QString description;
    };

    QMap<Page, Info> names{
        {Page::General,
         {tr("General"), tr("This page contains general program settings")}},

        {Page::Recognition,
         {tr("Recognition"),
          tr("This page contains text recognition settings. "
             "It shows the available languages that program can convert from "
             "image to text")}},

        {Page::Correction,
         {tr("Correction"),
          tr("This page contains recognized text correction settings. "
             "It allows to fix some errors after recognition.\n"
             "Hunspell searches for words that are similar to recognized ones "
             "in its dictionary.\n"
             "User correction allows to manually fix some frequently "
             "happening mistakes.\n"
             "User correction occurs before hunspell correction if both "
             "are enabled")}},

        {Page::Translation,
         {tr("Translation"),
          tr("This page contains settings, related to translation of the "
             "recognized text. "
             "Translation is done via enabled (checked) translation services. "
             "If one fails, then second one will be used and so on. "
             "If translator hangs it will be treated as failed after "
             "given timeout")}},

        {Page::Representation,
         {tr("Representation"),
          tr("This page contains result representation settings")}},

        {Page::Update,
         {tr("Update"),
          tr("This page allow to install/update/remove program resources")}},

        {Page::Help, {tr("Help"), tr("")}},
    };

    for (const auto &i : names) {
      const auto error = QString();
      pageModel_->appendRow({new QStandardItem(i.title),
                             new QStandardItem(i.description),
                             new QStandardItem(error)});
    }
    ui->pagesList->setModel(pageModel_);
    ui->pagesList->setModelColumn(int(PageColumn::Name));
    auto selection = ui->pagesList->selectionModel();
    connect(selection, &QItemSelectionModel::currentRowChanged,  //
            this, &SettingsEditor::updateCurrentPage);
    selection->select(pageModel_->index(0, 0),
                      QItemSelectionModel::SelectCurrent);
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
  ui->fontColor->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  ui->backgroundColor->setFocusPolicy(Qt::FocusPolicy::NoFocus);
#ifdef Q_OS_WINDOWS
  ui->fontColor->setFlat(true);
  ui->backgroundColor->setFlat(true);
#endif
  connect(ui->dialogRadio, &QRadioButton::toggled,  //
          ui->resultWindow, &QTableWidget::setEnabled);
  connect(ui->resultFont, &QFontComboBox::currentFontChanged,  //
          this, &SettingsEditor::updateResultFont);
  connect(ui->resultFontSize, qOverload<int>(&QSpinBox::valueChanged),  //
          this, &SettingsEditor::updateResultFont);
  connect(ui->fontColor, &QPushButton::clicked,  //
          this, [this] {
            pickColor(ui->fontColor);
            updateResultFont();
          });
  connect(ui->backgroundColor, &QPushButton::clicked,  //
          this, [this] {
            pickColor(ui->backgroundColor);
            updateResultFont();
          });

  // updates
  ui->updatesView->header()->setObjectName("updatesHeader");
  updater_.initView(ui->updatesView);
  connect(&updater_, &update::Updater::updated,  //
          this, &SettingsEditor::updateState);
  connect(ui->checkUpdates, &QPushButton::clicked,  //
          &updater_, &update::Updater::checkForUpdates);

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
  settings.translators = enabledTranslators();

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
  wasPortable_ = settings.isPortable();
  ui->portable->blockSignals(true);
  ui->portable->setChecked(settings.isPortable());
  ui->portable->blockSignals(false);

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
  updateModels();
  ui->tesseractLangCombo->setCurrentText(
      LanguageCodes::name(settings.sourceLanguage));
  ui->tesseractVersion->setCurrentIndex(int(settings.tesseractVersion));

  ui->useHunspell->setChecked(settings.useHunspell);
  ui->hunspellDir->setText(settings.hunspellPath);
  ui->useUserSubstitutions->setChecked(settings.useUserSubstitutions);
  ui->userSubstitutionsTable->setSubstitutions(settings.userSubstitutions);

  ui->doTranslationCheck->setChecked(settings.doTranslation);
  ui->ignoreSslCheck->setChecked(settings.ignoreSslErrors);
  ui->translateTimeoutSpin->setValue(settings.translationTimeout.count());
  ui->translatorsPath->setText(settings.translatorsPath);
  ui->translateLangCombo->setCurrentText(
      LanguageCodes::name(settings.targetLanguage));
  updateTranslators(settings.translators);

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

  updateState();
}

void SettingsEditor::updateState()
{
  Settings settings;
  settings.setPortable(ui->portable->isChecked());
  ui->tessdataPath->setText(settings.tessdataPath);
  ui->translatorsPath->setText(settings.translatorsPath);
  ui->hunspellDir->setText(settings.hunspellPath);

  updateModels();
  updateTranslators(enabledTranslators());
  validateSettings();
  updateCurrentPage();

  const auto portableChanged = wasPortable_ != settings.isPortable();
  ui->pageUpdate->setEnabled(!portableChanged);
  ui->pageUpdate->setToolTip(portableChanged
                                 ? tr("Portable changed. Apply settings first")
                                 : QString());
}

void SettingsEditor::updateCurrentPage()
{
  const auto row = ui->pagesList->currentIndex().row();

  const auto description = pageModel_->index(row, int(PageColumn::Description));
  ui->pageInfoLabel->setText(description.data().toString());
  ui->pageInfoLabel->setVisible(!ui->pageInfoLabel->text().isEmpty());

  const auto error = pageModel_->index(row, int(PageColumn::Error));
  ui->pageErrorLabel->setText(error.data().toString());
  ui->pageErrorLabel->setVisible(!ui->pageErrorLabel->text().isEmpty());

  ui->pagesView->setCurrentIndex(row);

  if (ui->pagesView->currentWidget() != ui->pageUpdate)
    return;

  if (ui->updatesView->model()->rowCount() == 0)
    updater_.checkForUpdates();
}

void SettingsEditor::updateTranslators(const QStringList &translators)
{
  ui->translatorList->clear();
  if (models_.translators().isEmpty())
    return;

  ui->translatorList->addItems(models_.translators());

  for (auto i = 0, end = ui->translatorList->count(); i < end; ++i) {
    auto item = ui->translatorList->item(i);
    item->setCheckState(translators.contains(item->text()) ? Qt::Checked
                                                           : Qt::Unchecked);
  }
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
    wasPortable_ = ui->portable->isChecked();
    updateState();
    return;
  }
}

void SettingsEditor::updateResultFont()
{
  auto font = ui->resultFont->currentFont();
  font.setPointSize(ui->resultFontSize->value());
  ui->backgroundColor->setFont(font);

  auto fontColor = ui->fontColor->palette().color(QPalette::Button);
  QPalette palette(ui->backgroundColor->palette());
  palette.setColor(QPalette::ButtonText, fontColor);
  ui->backgroundColor->setPalette(palette);
}

QStringList SettingsEditor::enabledTranslators() const
{
  QStringList result;
  for (auto i = 0, end = ui->translatorList->count(); i < end; ++i) {
    auto item = ui->translatorList->item(i);
    if (item->checkState() == Qt::Checked)
      result.append(item->text());
  }
  return result;
}

void SettingsEditor::updateModels()
{
  const auto source = ui->tesseractLangCombo->currentText();
  models_.update(ui->tessdataPath->text(), ui->translatorsPath->text());
  if (!source.isEmpty()) {
    ui->tesseractLangCombo->setCurrentText(source);
  } else if (ui->tesseractLangCombo->count() > 0) {
    ui->tesseractLangCombo->setCurrentIndex(0);
  }
}

void SettingsEditor::pickColor(QWidget *widget)
{
  const auto original = widget->palette().color(QPalette::Button);
  const auto color = QColorDialog::getColor(original, this);

  if (!color.isValid())
    return;

  QPalette palette(widget->palette());
  palette.setColor(QPalette::Button, color);
  widget->setPalette(palette);
}

void SettingsEditor::validateSettings()
{
  SettingsValidator validator;
  {
    auto settings = this->settings();
    if (validator.correct(settings, models_)) {
      setSettings(settings);
      return;
    }
  }

  for (auto i = 0, end = pageModel_->rowCount(); i < end; ++i) {
    const auto name = pageModel_->index(i, int(PageColumn::Name));
    pageModel_->setData(name, QBrush(Qt::black), Qt::ForegroundRole);

    const auto error = pageModel_->index(i, int(PageColumn::Error));
    pageModel_->setData(error, {});
  }

  const auto errors = validator.check(settings(), models_);
  if (errors.isEmpty())
    return;

  using E = SettingsValidator::Error;
  QMap<E, Page> errorToPage{
      {E::NoSourceInstalled, Page::Update},
      {E::NoSourceSet, Page::Recognition},
      {E::NoTranslatorInstalled, Page::Update},
      {E::NoTranslatorSet, Page::Translation},
      {E::NoTargetSet, Page::Translation},
  };

  QMap<Page, QStringList> summary;
  for (const auto err : errors) {
    SOFT_ASSERT(errorToPage.contains(err), continue);
    auto page = errorToPage[err];
    summary[page].push_back(validator.toString(err));
  }

  for (auto it = summary.cbegin(), end = summary.cend(); it != end; ++it) {
    const auto row = int(it.key());
    const auto index = pageModel_->index(row, int(PageColumn::Name));
    pageModel_->setData(index, QBrush(Qt::red), Qt::ForegroundRole);

    const auto error = pageModel_->index(row, int(PageColumn::Error));
    pageModel_->setData(error, it.value().join('\n'));
  }
}

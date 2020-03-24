#include "captureareaeditor.h"
#include "capturearea.h"
#include "captureareaselector.h"
#include "debug.h"
#include "languagecodes.h"
#include "settings.h"
#include "tesseract.h"
#include "translator.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QStringListModel>

CaptureAreaEditor::CaptureAreaEditor(CaptureAreaSelector &selector)
  : QWidget(&selector)
  , selector_(selector)
  , doTranslation_(new QCheckBox(tr("Translate"), this))
  , sourceLanguage_(new QComboBox(this))
  , targetLanguage_(new QComboBox(this))
  , sourceLanguageModel_(std::make_unique<QStringListModel>())
  , targetLanguageModel_(std::make_unique<QStringListModel>())
{
  setCursor(Qt::CursorShape::ArrowCursor);

  auto layout = new QFormLayout(this);
  layout->addRow(doTranslation_);
  layout->addRow(tr("Recognition language"), sourceLanguage_);
  layout->addRow(tr("Translation language"), targetLanguage_);

  sourceLanguage_->setModel(sourceLanguageModel_.get());
  targetLanguage_->setModel(targetLanguageModel_.get());
  targetLanguage_->setEnabled(doTranslation_->isChecked());

  connect(doTranslation_, &QCheckBox::toggled,  //
          targetLanguage_, &QComboBox::setEnabled);
}

CaptureAreaEditor::~CaptureAreaEditor() = default;

void CaptureAreaEditor::updateSettings(const Settings &settings)
{
  sourceLanguageModel_->setStringList(
      Tesseract::availableLanguageNames(settings.tessdataPath));
  targetLanguageModel_->setStringList(Translator::availableLanguageNames());
}

void CaptureAreaEditor::set(const CaptureArea &area)
{
  doTranslation_->setChecked(area.doTranslation_);
  sourceLanguage_->setCurrentText(LanguageCodes::name(area.sourceLanguage_));
  targetLanguage_->setCurrentText(LanguageCodes::name(area.targetLanguage_));
}

void CaptureAreaEditor::apply(CaptureArea &area) const
{
  area.doTranslation_ = doTranslation_->isChecked();
  area.sourceLanguage_ =
      LanguageCodes::idForName(sourceLanguage_->currentText());
  area.targetLanguage_ =
      LanguageCodes::idForName(targetLanguage_->currentText());
}

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
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
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

  auto layout = new QGridLayout(this);
  auto row = 0;
  layout->addWidget(doTranslation_, row, 0, 1, 2);

  ++row;
  layout->addWidget(new QLabel(tr("Recognition language:")), row, 0);
  layout->addWidget(sourceLanguage_, row, 1);
  auto swapLanguages = new QPushButton(tr("⇵"));
  layout->addWidget(swapLanguages, row, 2, 2, 1);

  ++row;
  layout->addWidget(new QLabel(tr("Translation language:")), row, 0);
  layout->addWidget(targetLanguage_, row, 1);

  sourceLanguage_->setModel(sourceLanguageModel_.get());
  targetLanguage_->setModel(targetLanguageModel_.get());
  targetLanguage_->setEnabled(doTranslation_->isChecked());

  swapLanguages->setFlat(true);
  {
    auto font = swapLanguages->font();
    font.setPointSize(std::max(font.pointSize() * 2, 16));
    swapLanguages->setFont(font);
  }
  swapLanguages->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  connect(doTranslation_, &QCheckBox::toggled,  //
          targetLanguage_, &QComboBox::setEnabled);
  connect(swapLanguages, &QPushButton::clicked,  //
          this, &CaptureAreaEditor::swapLanguages);
}

CaptureAreaEditor::~CaptureAreaEditor() = default;

void CaptureAreaEditor::updateSettings(const Settings &settings)
{
  sourceLanguageModel_->setStringList(
      Tesseract::availableLanguageNames(settings.tessdataPath));
  targetLanguageModel_->setStringList(Translator::availableLanguageNames());
}

void CaptureAreaEditor::swapLanguages()
{
  const auto target = targetLanguage_->currentText();
  targetLanguage_->setCurrentText(sourceLanguage_->currentText());
  sourceLanguage_->setCurrentText(target);
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

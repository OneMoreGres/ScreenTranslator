#include "captureareaeditor.h"
#include "capturearea.h"
#include "captureareaselector.h"
#include "commonmodels.h"
#include "languagecodes.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

CaptureAreaEditor::CaptureAreaEditor(const CommonModels &models,
                                     QWidget *parent)
  : QWidget(parent)
  , doTranslation_(new QCheckBox(tr("Translate:"), this))
  , isLocked_(new QCheckBox(tr("Save (can capture via hotkey)"), this))
  , sourceLanguage_(new QComboBox(this))
  , targetLanguage_(new QComboBox(this))
{
  setCursor(Qt::CursorShape::ArrowCursor);

  auto layout = new QGridLayout(this);
  auto row = 0;
  layout->addWidget(new QLabel(tr("Recognize:")), row, 0);
  layout->addWidget(sourceLanguage_, row, 1);
  auto swapLanguages = new QPushButton(tr("⇵"));
  layout->addWidget(swapLanguages, row, 2, 2, 1);

  ++row;
  layout->addWidget(doTranslation_, row, 0);
  layout->addWidget(targetLanguage_, row, 1);

  ++row;
  layout->addWidget(isLocked_, row, 0, 1, 2);

  sourceLanguage_->setModel(models.sourceLanguageModel());
  targetLanguage_->setModel(models.targetLanguageModel());
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

void CaptureAreaEditor::swapLanguages()
{
  const auto target = targetLanguage_->currentText();
  targetLanguage_->setCurrentText(sourceLanguage_->currentText());
  sourceLanguage_->setCurrentText(target);
}

void CaptureAreaEditor::set(const CaptureArea &area)
{
  isLocked_->setChecked(area.isLocked());
  doTranslation_->setChecked(area.doTranslation_);
  sourceLanguage_->setCurrentText(LanguageCodes::name(area.sourceLanguage_));
  targetLanguage_->setCurrentText(LanguageCodes::name(area.targetLanguage_));
}

void CaptureAreaEditor::apply(CaptureArea &area) const
{
  area.isLocked_ = isLocked_->isChecked();
  area.doTranslation_ = doTranslation_->isChecked();
  area.sourceLanguage_ =
      LanguageCodes::idForName(sourceLanguage_->currentText());
  area.targetLanguage_ =
      LanguageCodes::idForName(targetLanguage_->currentText());
}

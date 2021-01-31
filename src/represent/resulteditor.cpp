#include "resulteditor.h"
#include "commonmodels.h"
#include "debug.h"
#include "languagecodes.h"
#include "manager.h"
#include "settings.h"
#include "task.h"

#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>

ResultEditor::ResultEditor(Manager &manager, const CommonModels &models,
                           const Settings &settings, QWidget *parent)
  : QWidget(parent)
  , manager_(manager)
  , settings_(settings)
  , image_(new QLabel(this))
  , recognizedEdit_(new QTextEdit(this))
  , sourceLanguage_(new QComboBox(this))
  , targetLanguage_(new QComboBox(this))
  , recognizeOnly_(new QPushButton(tr("Recognize"), this))
  , recognizeAndTranslate_(new QPushButton(tr("Recognize and translate"), this))
  , translateOnly_(new QPushButton(tr("Translate"), this))
{
  image_->setAlignment(Qt::AlignCenter);

  sourceLanguage_->setModel(models.sourceLanguageModel());
  targetLanguage_->setModel(models.targetLanguageModel());

  connect(recognizeOnly_, &QPushButton::clicked,  //
          this, &ResultEditor::recognize);
  connect(recognizeAndTranslate_, &QPushButton::clicked,  //
          this, &ResultEditor::recognizeAndTranslate);
  connect(translateOnly_, &QPushButton::clicked,  //
          this, &ResultEditor::translate);

  auto layout = new QGridLayout(this);
  auto row = 0;
  layout->addWidget(image_, row, 0, 1, 2);

  ++row;
  layout->addWidget(new QLabel(tr("Recognize:")), row, 0);
  layout->addWidget(sourceLanguage_, row, 1);

  ++row;
  layout->addWidget(new QLabel(tr("Translate:")), row, 0);
  layout->addWidget(targetLanguage_, row, 1);

  ++row;
  layout->addWidget(recognizedEdit_, row, 0, 1, 2);

  ++row;
  auto box = new QHBoxLayout;
  layout->addLayout(box, row, 0, 1, 2);
  box->addWidget(recognizeOnly_);
  box->addWidget(recognizeAndTranslate_);
  box->addWidget(translateOnly_);
}

void ResultEditor::show(const TaskPtr &task)
{
  SOFT_ASSERT(task, return );
  task_ = std::make_shared<Task>();
  *task_ = *task;

  image_->setPixmap(task->captured);
  recognizedEdit_->setText(task->recognized);

  const auto target = task->targetLanguage.isEmpty() ? settings_.targetLanguage
                                                     : task->targetLanguage;
  targetLanguage_->setCurrentText(LanguageCodes::name(target));
  sourceLanguage_->setCurrentText(LanguageCodes::name(task->sourceLanguage));

  QWidget::show();
}

void ResultEditor::recognize()
{
  task_->sourceLanguage =
      LanguageCodes::idForName(sourceLanguage_->currentText());
  task_->targetLanguage.clear();
  manager_.captured(task_);
  close();
  task_.reset();
}

void ResultEditor::recognizeAndTranslate()
{
  task_->sourceLanguage =
      LanguageCodes::idForName(sourceLanguage_->currentText());
  task_->targetLanguage =
      LanguageCodes::idForName(targetLanguage_->currentText());
  task_->translators = settings_.translators;
  manager_.captured(task_);
  close();
  task_.reset();
}

void ResultEditor::translate()
{
  task_->targetLanguage =
      LanguageCodes::idForName(targetLanguage_->currentText());
  task_->translators = settings_.translators;
  task_->corrected = recognizedEdit_->toPlainText();
  manager_.corrected(task_);
  close();
  task_.reset();
}

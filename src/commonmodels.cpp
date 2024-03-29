#include "commonmodels.h"
#include "settings.h"
#include "tesseract.h"
#include "translator.h"

CommonModels::CommonModels()
  : sourceLanguageModel_(std::make_unique<QStringListModel>())
  , targetLanguageModel_(std::make_unique<QStringListModel>())
{
}

CommonModels::~CommonModels() = default;

void CommonModels::update(const QString &tessdataPath,
                          const QString &translatorPath)
{
  {
    auto names = Tesseract::availableLanguageNames(tessdataPath);
    std::sort(names.begin(), names.end());
    sourceLanguageModel_->setStringList(names);
  }

  {
    translators_ = Translator::availableTranslators(translatorPath);
    std::sort(translators_.begin(), translators_.end());
  }

  if (targetLanguageModel_->rowCount() > 0)
    return;

  {
    auto names = Translator::availableLanguageNames();
    std::sort(names.begin(), names.end());
    targetLanguageModel_->setStringList(names);
  }
}

QStringListModel *CommonModels::sourceLanguageModel() const
{
  return sourceLanguageModel_.get();
}

QStringListModel *CommonModels::targetLanguageModel() const
{
  return targetLanguageModel_.get();
}

const QStringList &CommonModels::translators() const
{
  return translators_;
}

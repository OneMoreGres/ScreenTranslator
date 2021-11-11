#pragma once

#include "stfwd.h"

#include <QStringListModel>

#include <memory>

class CommonModels
{
public:
  CommonModels();
  ~CommonModels();

  void update(const QString& tessdataPath, const QString& translatorPath);

  QStringListModel* sourceLanguageModel() const;
  QStringListModel* targetLanguageModel() const;
  const QStringList& translators() const;

private:
  std::unique_ptr<QStringListModel> sourceLanguageModel_;
  std::unique_ptr<QStringListModel> targetLanguageModel_;
  QStringList translators_;
};

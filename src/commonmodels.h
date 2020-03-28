#pragma once

#include "stfwd.h"

#include <QStringListModel>

#include <memory>

class CommonModels
{
public:
  CommonModels();
  ~CommonModels();

  void update(const QString& tessdataPath);

  QStringListModel* sourceLanguageModel() const;
  QStringListModel* targetLanguageModel() const;

private:
  std::unique_ptr<QStringListModel> sourceLanguageModel_;
  std::unique_ptr<QStringListModel> targetLanguageModel_;
};

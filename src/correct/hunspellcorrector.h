#pragma once

#include "stfwd.h"

#include <QString>

class Hunspell;

class HunspellCorrector
{
public:
  explicit HunspellCorrector(const LanguageId& language,
                             const QString& dictPath);
  ~HunspellCorrector();

  const QString& error() const;
  bool isValid() const;
  QString correct(const QString& original);

private:
  void init(const QString& path);
  void correctWord(QString& word, QTextCodec& codec) const;

  std::unique_ptr<Hunspell> engine_;
  QString error_;
};

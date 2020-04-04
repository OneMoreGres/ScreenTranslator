#pragma once

#include "stfwd.h"

#include <QPixmap>

class Task
{
public:
  bool isNull() const { return captured.isNull() && !sourceLanguage.isEmpty(); }
  bool isValid() const { return error.isEmpty(); }

  Generation generation{};

  QPoint capturePoint;
  QPixmap captured;
  QString recognized;
  QString corrected;
  QString translated;

  LanguageId sourceLanguage;
  LanguageId targetLanguage;

  QStringList translators;

  QString error;
  QStringList translatorErrors;
};

using TaskPtr = std::shared_ptr<Task>;

Q_DECLARE_METATYPE(TaskPtr);

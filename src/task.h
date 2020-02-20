#pragma once

#include "stfwd.h"

#include <QPixmap>

class Task
{
public:
  bool isNull() const { return captured.isNull() && !sourceLanguage.isEmpty(); }
  bool isValid() const { return error.isEmpty(); }
  //  void trace(const QString& message);

  QPoint capturePoint;
  QPixmap captured;
  QString recognized;
  QString translated{"sample"};

  LanguageId sourceLanguage{"eng"};
  LanguageId targetLanguage;  //{"ru"};

  QStringList translators{"google.js"};

  QString error;
  //  QStringList traceLog;

  //  bool swapLanguages;
};

using TaskPtr = std::shared_ptr<Task>;

Q_DECLARE_METATYPE(TaskPtr);

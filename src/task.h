#pragma once

#include "stfwd.h"

#include <QDebug>
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

  bool useHunspell{false};

  LanguageId sourceLanguage;
  LanguageId targetLanguage;

  QStringList translators;
  QString usedTranslator;

  QString error;
  QStringList translatorErrors;
};

using TaskPtr = std::shared_ptr<Task>;

Q_DECLARE_METATYPE(TaskPtr);

inline QDebug operator<<(QDebug debug, const TaskPtr &c)
{
  QDebugStateSaver saver(debug);
  debug.nospace() << "Task(Gen=" << c->generation
                  << ", pix=" << c->captured.size() << ", rec=" << c->recognized
                  << ", cor=" << c->corrected << ", tr=" << c->translated
                  << ", lang=" << qPrintable(c->sourceLanguage) << '-'
                  << qPrintable(c->targetLanguage) << ", err=" << c->error
                  << ')';

  return debug;
}

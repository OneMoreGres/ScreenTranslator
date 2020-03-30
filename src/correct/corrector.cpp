#include "corrector.h"
#include "debug.h"
#include "manager.h"
#include "settings.h"
#include "task.h"

Corrector::Corrector(Manager &manager, const Settings &settings)
  : manager_(manager)
  , settings_(settings)
{
}

void Corrector::correct(const TaskPtr &task)
{
  SOFT_ASSERT(task, return );
  SOFT_ASSERT(task->isValid(), return );

  if (task->recognized.isEmpty()) {
    manager_.corrected(task);
    return;
  }

  if (!settings_.userSubstitutions.empty())
    task->corrected = substituteUser(task->recognized, task->sourceLanguage);
  else
    task->corrected = task->recognized;

  manager_.corrected(task);
}

QString Corrector::substituteUser(const QString &source,
                                  const LanguageId &language) const
{
  auto result = source;

  const auto range = settings_.userSubstitutions.equal_range(language);
  if (range.first == settings_.userSubstitutions.cend())
    return result;

  while (true) {
    auto bestMatch = range.first;
    auto bestMatchLen = 0;

    for (auto it = range.first; it != range.second; ++it) {
      const auto &sub = it->second;
      if (!result.contains(sub.source))
        continue;
      const auto len = sub.source.length();
      if (len > bestMatchLen) {
        bestMatchLen = len;
        bestMatch = it;
      }
    }

    if (bestMatchLen < 1)
      break;

    result.replace(bestMatch->second.source, bestMatch->second.target);
  }

  return result;
}

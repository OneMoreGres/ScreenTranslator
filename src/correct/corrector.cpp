#include "corrector.h"
#include "debug.h"
#include "manager.h"
#include "task.h"

Corrector::Corrector(Manager &manager)
  : manager_(manager)
{
}

void Corrector::correct(const TaskPtr &task)
{
  SOFT_ASSERT(task, return );
  SOFT_ASSERT(task->isValid(), return );

  if (!userSubstitutions_.empty())
    task->recognized = substituteUser(task->recognized, task->sourceLanguage);

  manager_.corrected(task);
}

void Corrector::updateSettings(const Settings &settings)
{
  userSubstitutions_ = settings.userSubstitutions;
}

QString Corrector::substituteUser(const QString &source,
                                  const LanguageId &language) const
{
  auto result = source;

  const auto range = userSubstitutions_.equal_range(language);
  if (range.first == userSubstitutions_.cend())
    return result;

  while (true) {
    auto bestMatch = range.first;
    auto bestMatchLen = 0;

    for (auto it = range.first; it != range.second; ++it) {
      const auto &sub = it->second;
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

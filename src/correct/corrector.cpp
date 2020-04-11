#include "corrector.h"
#include "correctorworker.h"
#include "debug.h"
#include "languagecodes.h"
#include "manager.h"
#include "settings.h"
#include "task.h"

#include <QThread>

Corrector::Corrector(Manager &manager, const Settings &settings)
  : manager_(manager)
  , settings_(settings)
  , workerThread_(new QThread(this))
{
  auto worker = new CorrectorWorker;
  connect(this, &Corrector::resetAuto,  //
          worker, &CorrectorWorker::reset);
  connect(this, &Corrector::correctAuto,  //
          worker, &CorrectorWorker::handle);
  connect(worker, &CorrectorWorker::finished,  //
          this, &Corrector::finishCorrection);
  connect(workerThread_, &QThread::finished,  //
          worker, &QObject::deleteLater);

  workerThread_->start();
  worker->moveToThread(workerThread_);
}

Corrector::~Corrector()
{
  workerThread_->quit();
  const auto timeoutMs = 2000;
  if (!workerThread_->wait(timeoutMs))
    workerThread_->terminate();
}

void Corrector::correct(const TaskPtr &task)
{
  SOFT_ASSERT(task, return );
  SOFT_ASSERT(task->isValid(), return );

  if (task->recognized.isEmpty()) {
    manager_.corrected(task);
    return;
  }

  task->corrected = task->recognized;

  if (!settings_.userSubstitutions.empty())
    task->corrected = substituteUser(task->recognized, task->sourceLanguage);

  if (task->useHunspell) {
    emit correctAuto(task);
    return;
  }

  finishCorrection(task);
}

void Corrector::updateSettings()
{
  emit resetAuto(settings_.hunspellDir);
}

void Corrector::finishCorrection(const TaskPtr &task)
{
  manager_.corrected(task);
}

QString Corrector::substituteUser(const QString &source,
                                  const LanguageId &language) const
{
  auto result = source;

  using It = Substitutions::const_iterator;
  std::vector<std::pair<It, It>> ranges;

  {
    const auto range = settings_.userSubstitutions.equal_range(language);
    if (range.first != settings_.userSubstitutions.cend())
      ranges.push_back(range);
  }
  {
    const auto anyId = LanguageCodes::anyLanguageId();
    const auto range = settings_.userSubstitutions.equal_range(anyId);
    if (range.first != settings_.userSubstitutions.cend())
      ranges.push_back(range);
  }

  if (ranges.empty())
    return result;

  while (true) {
    auto bestMatch = ranges.front().first;
    auto bestMatchLen = 0;

    for (const auto &range : ranges) {
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
    }

    if (bestMatchLen < 1)
      break;

    result.replace(bestMatch->second.source, bestMatch->second.target);
  }

  return result;
}

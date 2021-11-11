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
  if (!workerThread_->wait(timeoutMs)) {
    LTRACE() << "terminating hunspell thread";
    workerThread_->terminate();
  }
}

void Corrector::correct(const TaskPtr &task)
{
  SOFT_ASSERT(task, return );
  SOFT_ASSERT(task->isValid(), return );

  queue_.push_back(task);

  if (task->recognized.isEmpty()) {
    finishCorrection(task);
    return;
  }

  task->corrected = task->recognized;

  if (!settings_.userSubstitutions.empty()) {
    task->corrected = substituteUser(task->recognized, task->sourceLanguage);
    LTRACE() << "Corrected with user data";
  }

  if (!task->useHunspell) {
    finishCorrection(task);
    return;
  }

  if (queue_.size() == 1)
    processQueue();
}

void Corrector::processQueue()
{
  if (queue_.empty())
    return;
  emit correctAuto(queue_.front());
}

void Corrector::updateSettings()
{
  queue_.clear();
  emit resetAuto(settings_.hunspellPath);
}

void Corrector::finishCorrection(const TaskPtr &task)
{
  manager_.corrected(task);

  SOFT_ASSERT(!queue_.empty(), return );
  if (queue_.front() == task) {
    queue_.pop_front();
  } else {
    LERROR() << "processed not first item in correction queue";
    queue_.clear();
  }
  processQueue();
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

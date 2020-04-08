#include "correctorworker.h"
#include "debug.h"
#include "hunspellcorrector.h"
#include "task.h"

CorrectorWorker::CorrectorWorker() = default;

CorrectorWorker::~CorrectorWorker() = default;

void CorrectorWorker::handle(const TaskPtr &task)
{
  SOFT_ASSERT(task, return );
  SOFT_ASSERT(task->isValid(), return );
  SOFT_ASSERT(!hunspellDir_.isEmpty(), return );

  auto result = task;

  if (!bundles_.count(task->sourceLanguage)) {
    auto engine =
        std::make_unique<HunspellCorrector>(task->sourceLanguage, hunspellDir_);

    if (!engine->isValid()) {
      LWARNING()
          << tr("Failed to init hunspell engine: %1").arg(engine->error());
      emit finished(result);
      return;
    }

    bundles_.emplace(task->sourceLanguage, Bundle{std::move(engine), 0});
  }

  auto &bundle = bundles_[task->sourceLanguage];
  SOFT_ASSERT(bundle.hunspell->isValid(), return );

  result->corrected = bundle.hunspell->correct(task->corrected);

  const auto keepGenerations = 10;
  bundle.usesLeft = keepGenerations;
  removeUnused(task->generation);
  lastGeneration_ = task->generation;

  emit finished(result);
}

void CorrectorWorker::reset(const QString &hunspellDir)
{
  if (hunspellDir_ == hunspellDir)
    return;

  hunspellDir_ = hunspellDir;
  bundles_.clear();
}

void CorrectorWorker::removeUnused(Generation current)
{
  if (lastGeneration_ == current)
    return;

  for (auto it = bundles_.begin(), end = bundles_.end(); it != end;) {
    if (it->second.usesLeft >= 0) {
      ++it;
    } else {
      it = bundles_.erase(it);
    }
  }
}

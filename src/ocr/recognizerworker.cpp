#include "recognizerworker.h"
#include "debug.h"
#include "task.h"
#include "tesseract.h"

RecognizeWorker::~RecognizeWorker() = default;

void RecognizeWorker::handle(const TaskPtr &task)
{
  SOFT_ASSERT(task, return );
  SOFT_ASSERT(task->isValid(), return );
  SOFT_ASSERT(!tessdataPath_.isEmpty(), return );

  auto result = task;

  if (!engines_.count(task->sourceLanguage)) {
    auto engine =
        std::make_unique<Tesseract>(task->sourceLanguage, tessdataPath_);

    if (!engine->isValid()) {
      result->error = tr("Failed to init OCR engine: %1").arg(engine->error());
      emit finished(result);
      return;
    }

    engines_.emplace(task->sourceLanguage, std::move(engine));
  }

  auto &engine = engines_[task->sourceLanguage];
  SOFT_ASSERT(engine->isValid(), return );

  result->recognized = engine->recognize(task->captured);
  if (result->recognized.isEmpty())
    result->error = engine->error();

  emit finished(result);
}

void RecognizeWorker::reset(const QString &tessdataPath)
{
  if (tessdataPath_ == tessdataPath)
    return;

  tessdataPath_ = tessdataPath;
  engines_.clear();
}

#include "recognizer.h"
#include "debug.h"
#include "manager.h"
#include "recognizerworker.h"
#include "settings.h"
#include "task.h"
#include "tesseract.h"

#include <QThread>

Recognizer::Recognizer(Manager &manager, const Settings &settings)
  : manager_(manager)
  , settings_(settings)
  , workerThread_(new QThread(this))
{
  auto worker = new RecognizeWorker;
  connect(this, &Recognizer::reset,  //
          worker, &RecognizeWorker::reset);
  connect(this, &Recognizer::recognizeImpl,  //
          worker, &RecognizeWorker::handle);
  connect(worker, &RecognizeWorker::finished,  //
          this, &Recognizer::recognized);
  connect(workerThread_, &QThread::finished,  //
          worker, &QObject::deleteLater);

  workerThread_->start();
  worker->moveToThread(workerThread_);
}

void Recognizer::recognize(const TaskPtr &task)
{
  SOFT_ASSERT(task, return );
  SOFT_ASSERT(task->isValid(), return );

  if (task->sourceLanguage.isEmpty()) {
    task->error = tr("No source language set. Check settings");
    manager_.recognized(task);
    return;
  }

  queue_.push_back(task);
  if (queue_.size() == 1)
    processQueue();
}

void Recognizer::processQueue()
{
  if (queue_.empty())
    return;
  emit recognizeImpl(queue_.front());
}

void Recognizer::recognized(const TaskPtr &task)
{
  manager_.recognized(task);

  SOFT_ASSERT(!queue_.empty(), return );
  if (queue_.front() == task) {
    queue_.pop_front();
  } else {
    LERROR() << "processed not first item in recognition queue";
    queue_.clear();
  }
  processQueue();
}

Recognizer::~Recognizer()
{
  workerThread_->quit();
  const auto timeoutMs = 2000;
  if (!workerThread_->wait(timeoutMs)) {
    LTRACE() << "terminating tesseract thread";
    workerThread_->terminate();
  }
}

void Recognizer::updateSettings()
{
  SOFT_ASSERT(!settings_.tessdataPath.isEmpty(), return );

  queue_.clear();
  const auto libName =
      (settings_.tesseractVersion == TesseractVersion::Optimized
           ? "tesseract-optimized"
           : "tesseract-compatible");
  emit reset(settings_.tessdataPath, libName);
}

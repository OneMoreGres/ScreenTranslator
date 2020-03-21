#include "recognizer.h"
#include "manager.h"
#include "recognizerworker.h"
#include "settings.h"
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
  connect(this, &Recognizer::recognize,  //
          worker, &RecognizeWorker::handle);
  connect(worker, &RecognizeWorker::finished,  //
          this, &Recognizer::recognized);
  connect(workerThread_, &QThread::finished,  //
          worker, &QObject::deleteLater);

  workerThread_->start();
  worker->moveToThread(workerThread_);
}

void Recognizer::recognized(const TaskPtr &task)
{
  manager_.recognized(task);
}

Recognizer::~Recognizer()
{
  workerThread_->quit();
  const auto timeoutMs = 2000;
  if (!workerThread_->wait(timeoutMs))
    workerThread_->terminate();
}

void Recognizer::updateSettings()
{
  if (settings_.tessdataPath.isEmpty()) {
    manager_.fatalError(tr("Tessdata path is empty"));
    return;
  }

  emit reset(settings_.tessdataPath);
}

#pragma once

#include "stfwd.h"

#include <QObject>

class Tesseract;

class RecognizeWorker : public QObject
{
  Q_OBJECT
public:
  ~RecognizeWorker();

  void handle(const TaskPtr &task);
  void reset(const QString &tessdataPath);

signals:
  void finished(const TaskPtr &task);

private:
  std::map<QString, std::unique_ptr<Tesseract>> engines_;
  QString tessdataPath_;
};

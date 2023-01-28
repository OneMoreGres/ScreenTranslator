#pragma once

#include "stfwd.h"

#include <QObject>

Q_MOC_INCLUDE("tesseract.h")

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
  void removeUnused(Generation current);

  std::map<QString, std::unique_ptr<Tesseract>> engines_;
  std::map<QString, Generation> lastGenerations_;
  QString tessdataPath_;
};

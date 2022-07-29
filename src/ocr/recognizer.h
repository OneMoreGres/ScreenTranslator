#pragma once

#include "stfwd.h"

#include <QObject>

#include <deque>

class Recognizer : public QObject
{
  Q_OBJECT
public:
  Recognizer(Manager &manager, const Settings &settings);
  ~Recognizer();

  void updateSettings();
  void recognize(const TaskPtr &task);

signals:
  void recognizeImpl(const TaskPtr &task);
  void reset(const QString &tessdataPath);

private:
  void recognized(const TaskPtr &task);
  void processQueue();

  Manager &manager_;
  const Settings &settings_;
  QThread *workerThread_;
  std::deque<TaskPtr> queue_;
};

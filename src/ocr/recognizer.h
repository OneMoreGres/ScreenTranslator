#pragma once

#include "stfwd.h"

#include <QObject>

class Recognizer : public QObject
{
  Q_OBJECT
public:
  Recognizer(Manager &manager, const Settings &settings);
  ~Recognizer();

  void updateSettings();

signals:
  void recognize(const TaskPtr &task);
  void reset(const QString &tessdataPath);

private:
  void recognized(const TaskPtr &task);

  Manager &manager_;
  const Settings &settings_;
  QThread *workerThread_;
};

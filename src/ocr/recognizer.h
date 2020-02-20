#pragma once

#include "stfwd.h"

#include <QObject>

class Recognizer : public QObject
{
  Q_OBJECT
public:
  explicit Recognizer(Manager &manager);
  ~Recognizer();

  void updateSettings(const Settings &settings);

signals:
  void recognize(const TaskPtr &task);
  void reset(const QString &tessdataPath);

private:
  void recognized(const TaskPtr &task);

  Manager &manager_;
  QThread *workerThread_;
};

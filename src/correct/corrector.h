#pragma once

#include "stfwd.h"

#include <QObject>

class Corrector : public QObject
{
  Q_OBJECT
public:
  Corrector(Manager &manager, const Settings &settings);
  ~Corrector();

  void correct(const TaskPtr &task);
  void updateSettings();

signals:
  void correctAuto(const TaskPtr &task);
  void resetAuto(const QString &tessdataPath);

private:
  void finishCorrection(const TaskPtr &task);
  QString substituteUser(const QString &source,
                         const LanguageId &language) const;

  Manager &manager_;
  const Settings &settings_;
  QThread *workerThread_;
};

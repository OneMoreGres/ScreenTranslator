#pragma once

#include "stfwd.h"

#include <QObject>

class HunspellCorrector;

class CorrectorWorker : public QObject
{
  Q_OBJECT
public:
  CorrectorWorker();
  ~CorrectorWorker();

  void handle(const TaskPtr &task);
  void reset(const QString &hunspellDir);

signals:
  void finished(const TaskPtr &task);

private:
  struct Bundle {
    std::unique_ptr<HunspellCorrector> hunspell;
    int usesLeft;
  };
  void removeUnused(Generation current);

  std::map<QString, Bundle> bundles_;
  Generation lastGeneration_{};
  QString hunspellDir_;
};

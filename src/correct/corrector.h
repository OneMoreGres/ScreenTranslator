#pragma once

#include "stfwd.h"

class Corrector
{
public:
  Corrector(Manager &manager, const Settings &settings);

  void correct(const TaskPtr &task);
  void updateSettings();

private:
  QString substituteUser(const QString &source,
                         const LanguageId &language) const;

  Manager &manager_;
  const Settings &settings_;
};

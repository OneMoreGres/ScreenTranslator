#pragma once

#include "settings.h"
#include "stfwd.h"

class Corrector
{
public:
  explicit Corrector(Manager &manager);

  void correct(const TaskPtr &task);
  void updateSettings(const Settings &settings);

private:
  QString substituteUser(const QString &source,
                         const LanguageId &language) const;

  Manager &manager_;
  Substitutions userSubstitutions_;
};

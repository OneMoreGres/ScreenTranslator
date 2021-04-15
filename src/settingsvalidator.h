#pragma once

#include <stfwd.h>

#include <QVector>

class SettingsValidator
{
public:
  enum class Error {
    NoSourceInstalled,
    NoSourceSet,
    NoTranslatorSet,
    NoTranslatorInstalled,
    NoTargetSet
  };

  QVector<Error> check(const Settings& settings,
                       const CommonModels& models) const;
  QString toString(Error error) const;
};

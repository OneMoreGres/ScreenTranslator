#include "settingsvalidator.h"

#include <commonmodels.h>
#include <settings.h>

QVector<SettingsValidator::Error> SettingsValidator::check(
    const Settings &settings, const CommonModels &models) const
{
  QVector<SettingsValidator::Error> result;

  if (models.sourceLanguageModel()->rowCount() == 0)
    result.append(Error::NoSourceInstalled);

  if (settings.sourceLanguage.isEmpty())
    result.append(Error::NoSourceSet);

  if (settings.doTranslation && models.targetLanguageModel()->rowCount() == 0)
    result.append(Error::NoTranslatorInstalled);

  if (settings.doTranslation && settings.translators.isEmpty())
    result.append(Error::NoTranslatorSet);

  if (settings.doTranslation && settings.targetLanguage.isEmpty())
    result.append(Error::NoTargetSet);

  return result;
}

QString SettingsValidator::toString(Error error) const
{
  switch (error) {
    case Error::NoSourceInstalled:
      return QObject::tr("No recognizers installed");

    case Error::NoSourceSet: return QObject::tr("Recognition language not set");

    case Error::NoTranslatorInstalled:
      return QObject::tr("No translators installed");

    case Error::NoTranslatorSet:
      return QObject::tr("No translators enabled (selected)");

    case Error::NoTargetSet: return QObject::tr("Translation language not set");
  }
  return {};
}

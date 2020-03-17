#include "settings.h"

#include <QFile>
#include <QSettings>
#include <QStandardPaths>

namespace
{
const QString iniFileName = "settings.ini";

const QString qs_guiGroup = "GUI";
const QString qs_captureHotkey = "captureHotkey";
const QString qs_repeatCaptureHotkey = "repeatCaptureHotkey";
const QString qs_repeatHotkey = "repeatHotkey";
const QString qs_clipboardHotkey = "clipboardHotkey";
const QString qs_resultShowType = "resultShowType";
const QString qs_proxyType = "proxyType";
const QString qs_proxyHostName = "proxyHostName";
const QString qs_proxyPort = "proxyPort";
const QString qs_proxyUser = "proxyUser";
const QString qs_proxyPassword = "proxyPassword";
const QString qs_proxySavePassword = "proxySavePassword";
const QString qs_autoUpdateType = "autoUpdateType";
const QString qs_lastUpdateCheck = "lastUpdateCheck";
const QString qs_showMessageOnStart = "showMessageOnStart";

const QString qs_recogntionGroup = "Recognition";
const QString qs_ocrLanguage = "language";

const QString qs_correctionGroup = "Correction";
const QString qs_userSubstitutions = "userSubstitutions";
const QString qs_useUserSubstitutions = "useUserSubstitutions";

const QString qs_translationGroup = "Translation";
const QString qs_doTranslation = "doTranslation";
const QString qs_ignoreSslErrors = "ignoreSslErrors";
const QString qs_translationLanguage = "translation_language";
const QString qs_translationTimeout = "translation_timeout";
const QString qs_debugMode = "translation_debug";
const QString qs_translators = "translators";

QString shuffle(const QString& source)
{
  if (source.isEmpty()) {
    return source;
  }
  char encKeys[] = {14, 26, 99, 43};
  std::string result = source.toStdString();
  for (size_t i = 0, end = result.size(); i < end; ++i) {
    result[i] = result[i] ^ encKeys[i % sizeof(encKeys)];
  }
  return QString::fromUtf8(result.data());
}

QStringList packSubstitutions(const Substitutions& source)
{
  QStringList result;
  for (const auto& i : source) {
    result << i.first << i.second.source << i.second.target;
  }
  return result;
}

Substitutions unpackSubstitutions(const QStringList& raw)
{
  const auto count = raw.size();
  if (count < 3)
    return {};

  Substitutions result;
  for (auto i = 0, end = raw.size(); i < end; i += 3) {
    result.emplace(raw[i], Substitution{raw[i + 1], raw[i + 2]});
  }
  return result;
}

Substitutions loadLegacySubstitutions()
{
  Substitutions result;

  QFile f("st_subs.csv");
  if (!f.open(QFile::ReadOnly))
    return result;

  const auto data = f.readAll();

  const auto lines =
      QString::fromUtf8(data).split('\n', QString::SkipEmptyParts);
  for (const auto& line : lines) {
    const auto parts = line.mid(1, line.size() - 2).split("\",\"");  // remove "
    if (parts.size() < 3)
      continue;
    result.emplace(parts[0], Substitution{parts[1], parts[2]});
  }
  return result;
}

void cleanupOutdated(QSettings& settings)
{
  if (!settings.contains(qs_recogntionGroup + "/image_scale"))
    return;

  settings.beginGroup(qs_guiGroup);
  settings.remove("geometry");
  settings.endGroup();

  settings.beginGroup(qs_recogntionGroup);
  settings.remove("image_scale");
  settings.remove("tessdata_dir");
  settings.endGroup();

  settings.beginGroup(qs_translationGroup);
  settings.remove("source_language");
  settings.remove("forceRotateTranslators");
  settings.endGroup();
}

}  // namespace

void Settings::save() const
{
  std::unique_ptr<QSettings> ptr;
  if (isPortable_) {
    ptr = std::make_unique<QSettings>(iniFileName, QSettings::IniFormat);
  } else {
    ptr = std::make_unique<QSettings>();
    QFile::remove(iniFileName);
  }
  auto& settings = *ptr;

  settings.beginGroup(qs_guiGroup);

  settings.setValue(qs_captureHotkey, captureHotkey);
  settings.setValue(qs_repeatCaptureHotkey, repeatCaptureHotkey);
  settings.setValue(qs_repeatHotkey, showLastHotkey);
  settings.setValue(qs_clipboardHotkey, clipboardHotkey);

  settings.setValue(qs_showMessageOnStart, showMessageOnStart);

  settings.setValue(qs_resultShowType, int(resultShowType));

  settings.setValue(qs_proxyType, int(proxyType));
  settings.setValue(qs_proxyHostName, proxyHostName);
  settings.setValue(qs_proxyPort, proxyPort);
  settings.setValue(qs_proxyUser, proxyUser);
  settings.setValue(qs_proxySavePassword, proxySavePassword);
  if (proxySavePassword) {
    settings.setValue(qs_proxyPassword, shuffle(proxyPassword));
  } else {
    settings.remove(qs_proxyPassword);
  }

  settings.setValue(qs_autoUpdateType, int(autoUpdateType));

  settings.endGroup();

  settings.beginGroup(qs_recogntionGroup);
  settings.setValue(qs_ocrLanguage, sourceLanguage);
  settings.endGroup();

  settings.beginGroup(qs_correctionGroup);
  settings.setValue(qs_useUserSubstitutions, useUserSubstitutions);
  settings.setValue(qs_userSubstitutions, packSubstitutions(userSubstitutions));
  settings.endGroup();

  settings.beginGroup(qs_translationGroup);

  settings.setValue(qs_doTranslation, doTranslation);
  settings.setValue(qs_ignoreSslErrors, ignoreSslErrors);
  settings.setValue(qs_debugMode, debugMode);
  settings.setValue(qs_translationLanguage, targetLanguage);
  settings.setValue(qs_translationTimeout, int(translationTimeout.count()));
  settings.setValue(qs_translators, translators);

  settings.endGroup();

  cleanupOutdated(settings);
}

void Settings::load()
{
  std::unique_ptr<QSettings> ptr;
  if (QFile::exists(iniFileName)) {
    ptr = std::make_unique<QSettings>(iniFileName, QSettings::IniFormat);
    setPortable(true);
  } else {
    ptr = std::make_unique<QSettings>();
    setPortable(false);
  }
  auto& settings = *ptr;

  settings.beginGroup(qs_guiGroup);

  captureHotkey = settings.value(qs_captureHotkey, captureHotkey).toString();
  repeatCaptureHotkey =
      settings.value(qs_repeatCaptureHotkey, repeatCaptureHotkey).toString();
  showLastHotkey = settings.value(qs_repeatHotkey, showLastHotkey).toString();
  clipboardHotkey =
      settings.value(qs_clipboardHotkey, clipboardHotkey).toString();

  showMessageOnStart =
      settings.value(qs_showMessageOnStart, showMessageOnStart).toBool();

  resultShowType = ResultMode(
      std::clamp(settings.value(qs_resultShowType, int(resultShowType)).toInt(),
                 int(ResultMode::Widget), int(ResultMode::Tooltip)));

  proxyType =
      ProxyType(std::clamp(settings.value(qs_proxyType, int(proxyType)).toInt(),
                           int(ProxyType::Disabled), int(ProxyType::Http)));
  proxyHostName = settings.value(qs_proxyHostName, proxyHostName).toString();
  proxyPort = settings.value(qs_proxyPort, proxyPort).toInt();
  proxyUser = settings.value(qs_proxyUser, proxyUser).toString();
  proxySavePassword =
      settings.value(qs_proxySavePassword, proxySavePassword).toBool();
  proxyPassword = shuffle(settings.value(qs_proxyPassword).toString());

  autoUpdateType = AutoUpdate(
      std::clamp(settings.value(qs_autoUpdateType, int(autoUpdateType)).toInt(),
                 int(AutoUpdate::Disabled), int(AutoUpdate::Monthly)));

  settings.endGroup();

  settings.beginGroup(qs_recogntionGroup);
  sourceLanguage = settings.value(qs_ocrLanguage, sourceLanguage).toString();
  settings.endGroup();

  settings.beginGroup(qs_correctionGroup);
  useUserSubstitutions =
      settings.value(qs_useUserSubstitutions, useUserSubstitutions).toBool();
  userSubstitutions =
      unpackSubstitutions(settings.value(qs_userSubstitutions).toStringList());
  if (userSubstitutions.empty())
    userSubstitutions = loadLegacySubstitutions();
  settings.endGroup();

  settings.beginGroup(qs_translationGroup);

  doTranslation = settings.value(qs_doTranslation, doTranslation).toBool();
  ignoreSslErrors =
      settings.value(qs_ignoreSslErrors, ignoreSslErrors).toBool();
  debugMode = settings.value(qs_debugMode, debugMode).toBool();
  targetLanguage =
      settings.value(qs_translationLanguage, targetLanguage).toString();
  translationTimeout = std::chrono::seconds(
      settings.value(qs_translationTimeout, int(translationTimeout.count()))
          .toInt());
  translators = settings.value(qs_translators, translators).toStringList();
  if (translators.size() == 1 && translators.first().contains('|'))  // legacy
    translators = translators.first().split('|');

  settings.endGroup();
}

bool Settings::isPortable() const
{
  return isPortable_;
}

void Settings::setPortable(bool isPortable)
{
  isPortable_ = isPortable;

  const auto baseDataPath =
      isPortable
          ? "."
          : QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  tessdataPath = baseDataPath + "/tessdata";
  translatorsDir = baseDataPath + "/translators";
}

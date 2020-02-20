#include "apptranslator.h"
#include "debug.h"

#include <QApplication>
#include <QDir>
#include <QLibraryInfo>
#include <QTranslator>

namespace
{
// const auto builtin = QLocale::English;
}  // namespace

AppTranslator::AppTranslator(const QStringList &translationFiles)
  : translationFiles_(translationFiles)
{
}

void AppTranslator::retranslate()
{
  auto app = QCoreApplication::instance();
  const auto oldTranslators = app->findChildren<QTranslator *>();
  for (const auto &old : oldTranslators) {
    old->deleteLater();
  }

  //  QLocale locale(translation());
  QLocale locale;
  const auto files =
      QStringList{QStringLiteral("qt"), QStringLiteral("qtbase")} +
      translationFiles_;
  const auto paths = searchPaths();

  auto last = new QTranslator(app);
  for (const auto &name : files) {
    for (const auto &path : paths) {
      if (!last->load(locale, name, QLatin1String("_"), path))
        continue;
      app->installTranslator(last);
      last = new QTranslator(app);
      break;
    }
  }
  last->deleteLater();
}

// QStringList TranslationLoader::availableLanguages()
//{
//  QStringList result{QLocale(builtin).nativeLanguageName()};
//  auto checker = new QTranslator(QApplication::instance());

//  for (const auto &dir : searchPaths()) {
//    for (const auto &file :
//         QDir(dir).entryInfoList({appTranslation + '*'}, QDir::Files)) {
//      if (checker->load(file.absoluteFilePath())) {
//        const auto name = file.baseName();
//        const auto suffixIndex = name.indexOf(QLatin1Char('_'));
//        if (suffixIndex < 0) {
//          continue;
//        }
//        const auto suffix = name.mid(suffixIndex + 1);
//        const auto locale = QLocale(suffix);
//        const auto language = locale.nativeLanguageName();
//        if (!result.contains(language)) {
//          result.append(language);
//        }
//      }
//    }
//  }
//  return result;
//}

// QString TranslationLoader::language()
//{
//  return toLanguage(translation());
//}

// void TranslationLoader::setLanguage(const QString &language)
//{
//  setTranslation(toTranslation(language));
//}

// QString TranslationLoader::translation()
//{
//  SettingsManager settings;
//  auto name = settings.get(SettingsManager::Translation).toString();
//  if (name.isEmpty()) {
//    const QLocale locale;
//    if (locale.language() == QLocale::Language::C) {
//      name = QLocale(builtin).name();
//    } else {
//      name = locale.name();
//    }
//  }
//  return name;
//}

// void TranslationLoader::setTranslation(const QString &translation)
//{
//  SettingsManager settings;
//  settings.set(SettingsManager::Translation, translation);
//}

QStringList AppTranslator::searchPaths() const
{
  return QStringList{
      QLibraryInfo::location(QLibraryInfo::TranslationsPath),
#ifdef Q_OS_LINUX
      qgetenv("APPDIR") +
          QLibraryInfo::location(QLibraryInfo::TranslationsPath),  // appimage
#endif  // ifdef Q_OS_LINUX
      {},
      QLatin1String("translations"),
  };
}

// QString TranslationLoader::toLanguage(const QString &translation)
//{
//  return QLocale(translation).nativeLanguageName();
//}

// QString TranslationLoader::toTranslation(const QString &language)
//{
//  for (auto i = 0; i < QLocale::Language::LastLanguage; ++i) {
//    const auto locale = QLocale(QLocale::Language(i));
//    if (locale.nativeLanguageName() == language) {
//      return locale.name();
//    }
//  }
//  return QLocale().name();
//}

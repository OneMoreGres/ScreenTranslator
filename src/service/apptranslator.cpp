#include "apptranslator.h"

#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>

namespace service
{
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
      QLatin1String(":/translations"),
  };
}

}  // namespace service

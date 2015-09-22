#include <QApplication>
#include <QTranslator>

#include <Manager.h>
#include <Settings.h>

int main (int argc, char *argv[]) {
  QApplication a (argc, argv);
  a.setQuitOnLastWindowClosed (false);
  a.setApplicationName (settings_values::appName);
  a.setOrganizationName (settings_values::companyName);

  QTranslator translator;
  // Set default to english.
  if (translator.load (QLocale::system (), "translation", "_", ":/translations") ||
      translator.load (":/translations/translation_en")) {
    a.installTranslator (&translator);
  }

  Manager manager;

  return a.exec ();
}

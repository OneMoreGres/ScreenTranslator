#include <QApplication>
#include <QTranslator>

#include <qtsingleapplication.h>

#include <Manager.h>
#include <Settings.h>

int main (int argc, char *argv[]) {
  QtSingleApplication a (argc, argv);
  if (a.sendMessage (QString ())) {
    return 0;
  }
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

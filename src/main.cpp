#ifdef Q_OS_LINUX
#include <locale.h>
#endif

#include "apptranslator.h"
#include "manager.h"
#include "singleapplication.h"

#include <QApplication>
#include <QCommandLineParser>

#define STR2(XXX) #XXX
#define STR(XXX) STR2(XXX)

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  a.setApplicationName("ScreenTranslator");
  a.setOrganizationName("Gres");
  a.setApplicationVersion(STR(VERSION));

  a.setQuitOnLastWindowClosed(false);

  {
    AppTranslator appTranslator({"screentranslator"});
    appTranslator.retranslate();
  }

  {
    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("OCR and translation tool"));
    parser.addHelpOption();
    parser.addVersionOption();

    parser.process(a);
  }

  SingleApplication guard;
  if (!guard.isValid())
    return 1;

  // tesseract recomments
  setlocale(LC_NUMERIC, "C");

  Manager manager;

  return a.exec();
}

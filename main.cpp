#include <QApplication>

#include <Manager.h>
#include <Settings.h>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  a.setQuitOnLastWindowClosed (false);//DEBUG
  a.setApplicationName (settings_values::appName);
  a.setOrganizationName (settings_values::companyName);

  Manager manager;

  return a.exec();
}

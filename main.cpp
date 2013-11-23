#include <QApplication>

#include <Manager.h>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
//  a.setQuitOnLastWindowClosed (false);//DEBUG

  Manager manager;

  return a.exec();
}

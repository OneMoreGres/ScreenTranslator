#include "runatsystemstart.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>

namespace service
{
#ifdef Q_OS_LINUX
QString desktopFile()
{
  auto name = QCoreApplication::applicationName().toLower();
  name.remove(QLatin1Char(' '));
  const auto result = QDir::homePath() + QLatin1String("/.config/autostart/") +
                      name + QLatin1String(".desktop");
  return result;
}

bool RunAtSystemStart::isAvailable()
{
  return true;
}

bool RunAtSystemStart::isEnabled()
{
  return QFile::exists(desktopFile());
}

void RunAtSystemStart::setEnabled(bool isOn)
{
  if (!isOn) {
    QFile::remove(desktopFile());
    return;
  }

  QFile f(desktopFile());
  if (!f.open(QFile::WriteOnly))
    return;

  const auto appPath =
      qEnvironmentVariable("APPIMAGE", QCoreApplication::applicationFilePath());
  const auto contents = QString(R"([Desktop Entry]
Name=%1
Exec=%2
)")
                            .arg(QCoreApplication::applicationName(), appPath);
  f.write(contents.toUtf8());
}
#endif

#ifdef Q_OS_WIN
const auto registryKey =
    "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";

bool RunAtSystemStart::isAvailable()
{
  return true;
}

bool RunAtSystemStart::isEnabled()
{
  QSettings settings(registryKey, QSettings::NativeFormat);
  return settings.contains(QCoreApplication::applicationName());
}

void RunAtSystemStart::setEnabled(bool isOn)
{
  QSettings settings(registryKey, QSettings::NativeFormat);
  if (isOn) {
    settings.setValue(
        QCoreApplication::applicationName(),
        QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
  } else {
    settings.remove(QCoreApplication::applicationName());
  }
}
#endif

#ifdef Q_OS_MAC
bool RunAtSystemStart::isAvailable()
{
  return false;
}

bool RunAtSystemStart::isEnabled()
{
  return false;
}

void RunAtSystemStart::setEnabled(bool /*isOn*/)
{
  return;
}
#endif

}  // namespace service

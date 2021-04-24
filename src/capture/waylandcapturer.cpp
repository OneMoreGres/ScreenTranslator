#include "waylandcapturer.h"
#include "debug.h"

#ifdef Q_OS_LINUX

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusPendingReply>
#include <QDir>
#include <QTemporaryFile>
#include <QUrl>

namespace
{
QString desktopFile()
{
  auto name = QCoreApplication::applicationName().toLower() +
              QLatin1String("-screenshot-permission");
  name.remove(QLatin1Char(' '));
  const auto result = QDir::homePath() +
                      QLatin1String("/.local/share/applications/") + name +
                      QLatin1String(".desktop");
  return result;
}

void removeDesktop()
{
  QFile::remove(desktopFile());
}

void writeDesktop()
{
  QFile f(desktopFile());
  if (!f.open(QFile::WriteOnly))
    return;

  const auto contents = QString(R"([Desktop Entry]
Name=%1-screen-permission
Exec=%2
X-KDE-DBUS-Restricted-Interfaces=org.kde.kwin.Screenshot
)")
                            .arg(QCoreApplication::applicationName(),
                                 QCoreApplication::applicationFilePath());
  f.write(contents.toUtf8());
}

}  // namespace

bool WaylandCapturerImpl::isWayland()
{
  return qEnvironmentVariable("XDG_SESSION_TYPE").toLower() ==
         QStringLiteral("wayland");
}

WaylandCapturerImpl::Method WaylandCapturerImpl::getMethod()
{
  auto de = qEnvironmentVariable("XDG_CURRENT_DESKTOP").toLower();
  if (de == QLatin1String("kde")) {
    return Method::Kde;
  } else if (de.endsWith(QLatin1String("gnome"))) {
    return Method::Gnome;
  }
  return Method::Freedesktop;
}

WaylandCapturerImpl::WaylandCapturerImpl()
  : method_(getMethod())
{
  if (method_ == Method::Kde)
    writeDesktop();
}

WaylandCapturerImpl::~WaylandCapturerImpl()
{
  if (method_ == Method::Kde)
    removeDesktop();
}

QPixmap WaylandCapturerImpl::grab()
{
  switch (method_) {
    case Method::Gnome: return grabGnome();
    case Method::Kde: return grabKde();
    case Method::Freedesktop: return grabFreedesktop();
  }
  return {};
}

QPixmap WaylandCapturerImpl::grabKde()
{
  auto request = QDBusMessage::createMethodCall(
      QStringLiteral("org.kde.KWin"), QStringLiteral("/Screenshot"),
      QStringLiteral("org.kde.kwin.Screenshot"),
      QStringLiteral("screenshotFullscreen"));

  request << false;

  auto reply = QDBusConnection::sessionBus().call(request);
  const auto args = reply.arguments();
  if (reply.type() == QDBusMessage::ErrorMessage || args.isEmpty()) {
    LERROR() << "kde capture error" << reply;
    return {};
  }

  const auto fileName = args.first().toString();
  auto result = QPixmap(fileName);
  QFile::remove(fileName);
  return result;
}

QPixmap WaylandCapturerImpl::grabGnome()
{
  auto request = QDBusMessage::createMethodCall(
      QStringLiteral("org.gnome.Shell.Screenshot"),
      QStringLiteral("/org/gnome/Shell/Screenshot"),
      QStringLiteral("org.gnome.Shell.Screenshot"),
      QStringLiteral("Screenshot"));

  QTemporaryFile f;
  if (!f.open()) {
    LERROR() << "failed to create temp file" << f.errorString();
    return {};
  }
  f.close();

  request << false << false << f.fileName();

  auto reply = QDBusConnection::sessionBus().call(request);
  const auto args = reply.arguments();
  if (reply.type() == QDBusMessage::ErrorMessage || args.isEmpty()) {
    LERROR() << "gnome capture error" << reply;
    return {};
  }

  if (!args.first().toBool()) {
    LERROR() << "gnome capture error";
    return {};
  }

  return QPixmap(f.fileName());
}

QPixmap WaylandCapturerImpl::grabFreedesktop()
{
  auto request = QDBusMessage::createMethodCall(
      QStringLiteral("org.freedesktop.portal.Desktop"),
      QStringLiteral("/org/freedesktop/portal/desktop"),
      QStringLiteral("org.freedesktop.portal.Screenshot"),
      QStringLiteral("Screenshot"));

  request << QStringLiteral("") << QVariantMap{};

  auto reply = QDBusConnection::sessionBus().call(request);
  const auto args = reply.arguments();
  if (reply.type() == QDBusMessage::ErrorMessage || args.isEmpty()) {
    LERROR() << "freedesktop capture error" << reply;
    return {};
  }

  const auto handleArg = args.first();
  if (!handleArg.canConvert<QDBusObjectPath>()) {
    LERROR() << "wrong type in freedesktop ansert" << handleArg.userType();
    return {};
  }

  const auto handle = handleArg.value<QDBusObjectPath>().path();

  QDBusConnection::sessionBus().connect(
      QStringLiteral("org.freedesktop.portal.Desktop"), handle,
      QStringLiteral("org.freedesktop.portal.Request"),
      QStringLiteral("Response"), this, SLOT(getScreen(uint, QVariantMap)));

  loop_.exec();
  return result_;
}

void WaylandCapturerImpl::parseFreedesktopResult(uint response,
                                                 const QVariantMap &results)
{
  if (response == 0) {
    const auto name = QUrl(results["uri"].toString()).toLocalFile();
    result_.load(name);
    QFile::remove(name);
  }
  loop_.exit();
}

#endif

std::unique_ptr<WaylandCapturer> WaylandCapturer::create()
{
#ifdef Q_OS_LINUX
  if (WaylandCapturerImpl::isWayland())
    return std::make_unique<WaylandCapturerImpl>();
#endif
  return {};
}

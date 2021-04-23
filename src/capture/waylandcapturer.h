#pragma once

#include <QEventLoop>
#include <QObject>
#include <QPixmap>

class WaylandCapturer : public QObject
{
  Q_OBJECT
public:
  WaylandCapturer();
  ~WaylandCapturer();

  static bool isWayland();

  QPixmap grab();

private slots:
  void parseFreedesktopResult(uint response, const QVariantMap &results);

private:
  enum class Method { Gnome, Kde, Freedesktop };
  static Method getMethod();

  QPixmap grabKde();
  QPixmap grabGnome();
  QPixmap grabFreedesktop();

  Method method_;
  QEventLoop loop_;
  QPixmap result_;
};

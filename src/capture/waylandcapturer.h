#pragma once

#include <QEventLoop>
#include <QObject>
#include <QPixmap>

class WaylandCapturer
{
public:
  virtual ~WaylandCapturer() = default;

  virtual QPixmap grab() = 0;

  static std::unique_ptr<WaylandCapturer> create();
};

#ifdef Q_OS_LINUX

class WaylandCapturerImpl : public QObject, public WaylandCapturer
{
  Q_OBJECT
public:
  WaylandCapturerImpl();
  ~WaylandCapturerImpl();

  static bool isWayland();

  QPixmap grab() override;

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

#endif

#pragma once

#include "stfwd.h"

#include <QPixmap>

class WaylandCapturer;

class Capturer
{
public:
  Capturer(Manager &manager, const Settings &settings,
           const CommonModels &models);
  ~Capturer();

  void capture();
  bool canCaptureLocked();
  void captureLocked();
  void repeatCapture();
  void updateSettings();

  void selected(const CaptureArea &area);
  void canceled();

private:
  void updatePixmap();

  Manager &manager_;
  const Settings &settings_;
  QPixmap pixmap_;
  std::unique_ptr<CaptureAreaSelector> selector_;
  std::unique_ptr<WaylandCapturer> wayland_;
};

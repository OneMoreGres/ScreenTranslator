#pragma once

#include "stfwd.h"

#include <QPixmap>

class Capturer
{
public:
  Capturer(Manager &manager, const Settings &settings);
  ~Capturer();

  void capture();
  void repeatCapture();
  void updateSettings();

  void captured(const TaskPtr &task);
  void canceled();

private:
  void updatePixmap();

  Manager &manager_;
  const Settings &settings_;
  QPixmap pixmap_;
  std::unique_ptr<CaptureAreaSelector> selector_;
};

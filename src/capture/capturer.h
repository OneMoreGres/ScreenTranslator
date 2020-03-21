#pragma once

#include "stfwd.h"

#include <vector>

class Capturer
{
public:
  Capturer(Manager &manager, const Settings &settings);

  void capture();
  void repeatCapture();
  void updateSettings();

  void captured(const TaskPtr &task);
  void canceled();

private:
  void showOverlays(bool capturePixmap);
  void hideOverlays();

  Manager &manager_;
  const Settings &settings_;
  std::vector<CaptureAreaSelector *> selectors_;
};

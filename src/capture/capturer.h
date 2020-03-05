#pragma once

#include "stfwd.h"

#include <QStringList>

#include <vector>

class CaptureOverlay;

class Capturer
{
public:
  explicit Capturer(Manager &manager);

  void capture();
  void repeatCapture();
  void updateSettings(const Settings &settings);

  void captured(const TaskPtr &task);
  void canceled();

private:
  void showOverlays(bool capturePixmap);
  void hideOverlays();

  Manager &manager_;
  std::vector<CaptureOverlay *> overlays_;

  LanguageId sourceLanguage_;
  LanguageId targetLanguage_;
  QStringList translators_;
  bool doTranslation_{false};
};

#pragma once

#include "stfwd.h"

#include <QStringList>

#include <chrono>

enum class ResultMode { Widget, Tooltip };

struct Substitution {
  QString source;
  QString target;
};
using Substitutions = std::unordered_multimap<LanguageId, Substitution>;

class Settings
{
public:
  void save();
  void load();

  QString captureHotkey{"Ctrl+Alt+Z"};
  QString repeatCaptureHotkey{"Ctrl+Alt+S"};
  QString showLastHotkey{"Ctrl+Alt+X"};
  QString clipboardHotkey{"Ctrl+Alt+C"};

  int proxyType{0};
  QString proxyHostName{""};
  int proxyPort{8080};
  QString proxyUser{""};
  QString proxyPassword{""};
  bool proxySavePassword{false};

  int autoUpdateType{0};  // Never
  QString lastUpdateCheck{""};
  Substitutions userSubstitutions;

  bool debugMode{false};

  QString tessdataPath{"tessdata"};
  QString sourceLanguage{"eng"};
  LanguageIds availableOcrLanguages_;

  bool doTranslation{true};
  bool ignoreSslErrors{false};
  bool forceRotateTranslators{false};
  LanguageId targetLanguage{"rus"};
  std::chrono::seconds translationTimeout{15};
  QString translatorsDir{"translators"};
  QStringList translators{"google.js"};

  ResultMode resultShowType{ResultMode::Widget};  // dialog
};

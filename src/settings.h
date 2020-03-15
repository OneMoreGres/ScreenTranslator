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

enum class ProxyType { Disabled, System, Socks5, Http };

enum class AutoUpdate { Disabled, Daily, Weekly, Monthly };

class Settings
{
public:
  void save() const;
  void load();

  QString captureHotkey{"Ctrl+Alt+Z"};
  QString repeatCaptureHotkey{"Ctrl+Alt+S"};
  QString showLastHotkey{"Ctrl+Alt+X"};
  QString clipboardHotkey{"Ctrl+Alt+C"};

  bool showMessageOnStart{true};

  ProxyType proxyType{ProxyType::System};
  QString proxyHostName;
  int proxyPort{8080};
  QString proxyUser;
  QString proxyPassword;
  bool proxySavePassword{false};

  AutoUpdate autoUpdateType{AutoUpdate::Disabled};
  QString lastUpdateCheck{""};

  Substitutions userSubstitutions;
  bool useUserSubstitutions{true};

  bool debugMode{false};

  QString tessdataPath;
  QString sourceLanguage{"eng"};
  LanguageIds availableOcrLanguages_;

  bool doTranslation{true};
  bool ignoreSslErrors{false};
  bool forceRotateTranslators{false};
  LanguageId targetLanguage{"rus"};
  std::chrono::seconds translationTimeout{15};
  QString translatorsDir;
  QStringList translators{"google.js"};

  ResultMode resultShowType{ResultMode::Widget};  // dialog
};

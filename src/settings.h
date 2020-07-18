#pragma once

#include "stfwd.h"

#include <QColor>
#include <QDateTime>
#include <QStringList>

#include <chrono>

enum class ResultMode { Widget, Tooltip };

struct Substitution {
  QString source;
  QString target;
};
using Substitutions = std::unordered_multimap<LanguageId, Substitution>;

enum class ProxyType { Disabled, System, Socks5, Http };

enum class TesseractVersion { Optimized, Compatible };

class Settings
{
public:
  void save() const;
  void load();

  void saveLastUpdateCheck();

  bool isPortable() const;
  void setPortable(bool isPortable);

  QString captureHotkey{"Ctrl+Alt+Z"};
  QString repeatCaptureHotkey{"Ctrl+Alt+S"};
  QString showLastHotkey{"Ctrl+Alt+X"};
  QString clipboardHotkey{"Ctrl+Alt+C"};
  QString captureLockedHotkey{"Ctrl+Alt+Q"};

  bool showMessageOnStart{true};
  bool runAtSystemStart{false};

  ProxyType proxyType{ProxyType::System};
  QString proxyHostName;
  int proxyPort{8080};
  QString proxyUser;
  QString proxyPassword;
  bool proxySavePassword{false};

  int autoUpdateIntervalDays{0};
  QDateTime lastUpdateCheck;

  bool useHunspell{false};
  QString hunspellDir;
  Substitutions userSubstitutions;
  bool useUserSubstitutions{true};

  bool writeTrace{false};

  QString tessdataPath;
  QString sourceLanguage{"eng"};
  TesseractVersion tesseractVersion{TesseractVersion::Optimized};

  bool doTranslation{true};
  bool ignoreSslErrors{false};
  LanguageId targetLanguage{"rus"};
  std::chrono::seconds translationTimeout{15};
  QString translatorsDir;
  QStringList translators{"google.js"};

  ResultMode resultShowType{ResultMode::Widget};  // dialog
  QString fontFamily;
  int fontSize{11};
  QColor fontColor{Qt::black};
  QColor backgroundColor{Qt::lightGray};
  bool showRecognized{true};
  bool showCaptured{true};

private:
  bool isPortable_{false};
};

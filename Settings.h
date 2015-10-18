#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>

namespace settings_names {
  //! UI
  const QString guiGroup = "GUI";
  const QString geometry = "geometry";
  const QString captureHotkey = "captureHotkey";
  const QString repeatCaptureHotkey = "repeatCaptureHotkey";
  const QString repeatHotkey = "repeatHotkey";
  const QString clipboardHotkey = "clipboardHotkey";
  const QString resultShowType = "resultShowType";
  const QString proxyType = "proxyType";
  const QString proxyHostName = "proxyHostName";
  const QString proxyPort = "proxyPort";
  const QString proxyUser = "proxyUser";
  const QString proxyPassword = "proxyPassword";
  const QString proxySavePassword = "proxySavePassword";
  const QString autoUpdateType = "autoUpdateType";
  const QString lastUpdateCheck = "lastUpdateCheck";

  //! Recognition
  const QString recogntionGroup = "Recognition";
  const QString tessDataPlace = "tessdata_dir";
  const QString ocrLanguage = "language";
  const QString imageScale = "image_scale";

  //! Translation
  const QString translationGroup = "Translation";
  const QString doTranslation = "doTranslation";
  const QString sourceLanguage = "source_language";
  const QString translationLanguage = "translation_language";
  const QString translationTimeout = "translation_timeout";
  const QString translationDebugMode = "translation_debug";
  const QString translators = "translators";
}

namespace settings_values {
  const QString appName = "ScreenTranslator";
  const QString companyName = "Gres";

  //! UI
  const QString captureHotkey = "Ctrl+Alt+Z";
  const QString repeatCaptureHotkey = "Ctrl+Alt+S";
  const QString repeatHotkey = "Ctrl+Alt+X";
  const QString clipboardHotkey = "Ctrl+Alt+C";
  const QString resultShowType = "1";//dialog
  const int proxyType = 0;
  const QString proxyHostName = "";
  const int proxyPort = 8080;
  const QString proxyUser = "";
  const QString proxyPassword = "";
  const bool proxySavePassword = false;
  const int autoUpdateType = 0; //Never
  const QString lastUpdateCheck = "";

  //! Recognition
  const QString tessDataPlace = "./";
  const QString ocrLanguage = "eng";
  const int imageScale = 5;

  //! Translation
  const bool doTranslation = true;
  const QString sourceLanguage = "auto";
  const QString translationLanguage = "ru";
  const int translationTimeout = 15; // secs
  const bool translationDebugMode = false;
  const QString translators = "";
}

#endif // SETTINGS_H

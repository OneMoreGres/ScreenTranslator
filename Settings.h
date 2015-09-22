#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>

namespace settings_names {
  //! UI
  const QString guiGroup = "GUI";
  const QString geometry = "geometry";
  const QString captureHotkey = "captureHotkey";
  const QString repeatHotkey = "repeatHotkey";
  const QString clipboardHotkey = "clipboardHotkey";
  const QString resultShowType = "resultShowType";

  //! Recognition
  const QString recogntionGroup = "Recognition";
  const QString tessDataPlace = "tessdata_dir";
  const QString ocrLanguage = "language";
  const QString imageScale = "image_scale";

  //! Translation
  const QString translationGroup = "Translation";
  const QString sourceLanguage = "source_language";
  const QString translationLanguage = "translation_language";

}

namespace settings_values {
  const QString appName = "ScreenTranslator";
  const QString companyName = "Gres";

  //! UI
  const QString captureHotkey = "Ctrl+Alt+Z";
  const QString repeatHotkey = "Ctrl+Alt+X";
  const QString clipboardHotkey = "Ctrl+Alt+C";
  const QString resultShowType = "1";//dialog

  //! Recognition
  const QString tessDataPlace = "./";
  const QString ocrLanguage = "eng";
  const int imageScale = 5;

  //! Translation
  const QString sourceLanguage = "auto";
  const QString translationLanguage = "ru";
}

#endif // SETTINGS_H

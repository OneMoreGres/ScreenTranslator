#pragma once

#include <QString>

#include <optional>
#include <unordered_map>

using LanguageId = QString;

class LanguageCodes
{
public:
  static LanguageId idForTesseract(const QString& tesseract);
  static LanguageId idForName(const QString& name);
  static QString iso639_1(const LanguageId& id);
  static QString tesseract(const LanguageId& id);
  static QString name(const LanguageId& id);
  static std::vector<LanguageId> allIds();
  static LanguageId anyLanguageId();

private:
  struct Bundle {
    LanguageId id;
    QString iso639_1;
    QString tesseract;
    const char* name;
  };

  LanguageCodes() = delete;
  LanguageCodes(const LanguageCodes&) = delete;
  LanguageCodes& operator=(const LanguageCodes&) = delete;

  const static std::unordered_map<LanguageId, Bundle> codes_;
};

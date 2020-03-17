#pragma once

#include <QString>

#include <optional>
#include <unordered_map>

using LanguageId = QString;

class LanguageCodes
{
public:
  struct Bundle {
    LanguageId id;
    QString iso639_1;
    QString tesseract;
    const char* name;
  };

  std::optional<Bundle> findById(const LanguageId& id) const;
  std::optional<Bundle> findByName(const QString& name) const;
  std::optional<Bundle> findByTesseract(const QString& name) const;
  const std::unordered_map<LanguageId, Bundle>& all() const;

private:
  const static std::unordered_map<LanguageId, Bundle> codes_;
};

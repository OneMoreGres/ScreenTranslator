#pragma once

#include <QStringList>

class AppTranslator
{
public:
  explicit AppTranslator(const QStringList &translationFiles);

  void retranslate();

  //  static QStringList availableLanguages();
  //  static QString language();
  //  static void setLanguage(const QString &language);

private:
  //  static QString translation();
  //  static void setTranslation(const QString &translation);
  QStringList searchPaths() const;

  //  static QString toTranslation(const QString &language);
  //  static QString toLanguage(const QString &translation);

  QStringList translationFiles_;
};

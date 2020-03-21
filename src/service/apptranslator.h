#pragma once

#include <QStringList>

class AppTranslator
{
public:
  explicit AppTranslator(const QStringList &translationFiles);

  void retranslate();

private:
  QStringList searchPaths() const;
  QStringList translationFiles_;
};

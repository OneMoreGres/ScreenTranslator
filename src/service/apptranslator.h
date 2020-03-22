#pragma once

#include <QStringList>

namespace service
{
class AppTranslator
{
public:
  explicit AppTranslator(const QStringList &translationFiles);

  void retranslate();

private:
  QStringList searchPaths() const;
  QStringList translationFiles_;
};

}  // namespace service

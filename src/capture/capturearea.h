#pragma once

#include "stfwd.h"

#include <QRect>
#include <QStringList>

class QPixmap;

class CaptureArea
{
public:
  CaptureArea(const QRect& rect, const Settings& settings);
  TaskPtr task(const QPixmap& pixmap) const;

private:
  QRect rect_;
  bool doTranslation_;
  LanguageId sourceLanguage_;
  LanguageId targetLanguage_;
  QStringList translators_;
};

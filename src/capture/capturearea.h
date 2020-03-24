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

  bool isValid() const;
  const QRect& rect() const;
  void setRect(const QRect& rect);

private:
  friend class CaptureAreaEditor;

  QRect rect_;
  bool doTranslation_;
  LanguageId sourceLanguage_;
  LanguageId targetLanguage_;
  QStringList translators_;
};

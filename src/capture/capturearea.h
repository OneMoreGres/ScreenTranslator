#pragma once

#include "stfwd.h"

#include <QRect>
#include <QStringList>

class QPixmap;

class CaptureArea
{
public:
  CaptureArea(const QRect& rect, const Settings& settings);
  TaskPtr task(const QPixmap& pixmap, const QPoint& pixmapOffset) const;

  void setGeneration(uint generation);
  bool isValid() const;
  bool isLocked() const;
  const QRect& rect() const;
  void setRect(const QRect& rect);

  QString toolTip() const;

private:
  friend class CaptureAreaEditor;

  Generation generation_{};
  QRect rect_;
  bool doTranslation_;
  bool isLocked_{false};
  bool useHunspell_{false};
  LanguageId sourceLanguage_;
  LanguageId targetLanguage_;
  QStringList translators_;
};

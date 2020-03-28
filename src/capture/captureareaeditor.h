#pragma once

#include "stfwd.h"

#include <QWidget>

class QCheckBox;
class QComboBox;

class CaptureAreaEditor : public QWidget
{
  Q_OBJECT
public:
  explicit CaptureAreaEditor(const CommonModels& models,
                             CaptureAreaSelector& selector);
  ~CaptureAreaEditor();

  void set(const CaptureArea& area);
  void apply(CaptureArea& area) const;

private:
  void swapLanguages();

  CaptureAreaSelector& selector_;
  QCheckBox* doTranslation_;
  QComboBox* sourceLanguage_;
  QComboBox* targetLanguage_;
};

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
                             QWidget* parent = nullptr);
  ~CaptureAreaEditor();

  void set(const CaptureArea& area);
  void apply(CaptureArea& area) const;

private:
  void swapLanguages();

  QCheckBox* doTranslation_;
  QCheckBox* isLocked_;
  QCheckBox* useHunspell_;
  QComboBox* sourceLanguage_;
  QComboBox* targetLanguage_;
};

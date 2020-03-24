#pragma once

#include "stfwd.h"

#include <QWidget>

class QCheckBox;
class QComboBox;
class QStringListModel;

class CaptureAreaEditor : public QWidget
{
  Q_OBJECT
public:
  explicit CaptureAreaEditor(CaptureAreaSelector& selector);
  ~CaptureAreaEditor();

  void set(const CaptureArea& area);
  void apply(CaptureArea& area) const;
  void updateSettings(const Settings& settings);

private:
  CaptureAreaSelector& selector_;
  QCheckBox* doTranslation_;
  QComboBox* sourceLanguage_;
  QComboBox* targetLanguage_;
  std::unique_ptr<QStringListModel> sourceLanguageModel_;
  std::unique_ptr<QStringListModel> targetLanguageModel_;
};

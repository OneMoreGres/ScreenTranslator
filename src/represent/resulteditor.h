#pragma once

#include "stfwd.h"

#include <QWidget>

class QLabel;
class QMenu;
class QTextEdit;
class QComboBox;
class QPushButton;

class ResultEditor : public QWidget
{
  Q_OBJECT
public:
  ResultEditor(Manager& manager, const CommonModels& models,
               const Settings& settings, QWidget* parent = nullptr);

  void show(const TaskPtr& task);
  using QWidget::show;

private:
  void recognize();
  void recognizeAndTranslate();
  void translate();

  Manager& manager_;
  const Settings& settings_;
  TaskPtr task_;
  QLabel* image_;
  QTextEdit* recognizedEdit_;
  QComboBox* sourceLanguage_;
  QComboBox* targetLanguage_;
  QPushButton* recognizeOnly_;
  QPushButton* recognizeAndTranslate_;
  QPushButton* translateOnly_;
};

#pragma once

#include "stfwd.h"

#include <QFrame>

class QLabel;

class ResultWidget : public QFrame
{
  Q_OBJECT
public:
  explicit ResultWidget(const Settings& settings, QWidget* parent = nullptr);

  void show(const TaskPtr& task);
  using QWidget::show;
  void updateSettings();

  bool eventFilter(QObject* watched, QEvent* event) override;

private:
  const Settings& settings_;
  QLabel* image_;
  QLabel* recognized_;
  QLabel* translated_;
};

#pragma once

#include "stfwd.h"

#include <QFrame>

class QLabel;

class ResultWidget : public QFrame
{
  Q_OBJECT
public:
  ResultWidget(QWidget* parent = nullptr);

  void show(const TaskPtr& task);
  using QWidget::show;
  void updateSettings(const QFont& font, bool showRecognized,
                      bool showCaptured);

  bool eventFilter(QObject* watched, QEvent* event) override;

private:
  QLabel* image_;
  QLabel* recognized_;
  QLabel* translated_;
  bool showRecognized_{true};
};

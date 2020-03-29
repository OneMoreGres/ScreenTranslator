#pragma once

#include "stfwd.h"

#include <QFrame>

class QLabel;
class QMenu;

class ResultWidget : public QFrame
{
  Q_OBJECT
public:
  ResultWidget(Representer& representer, const Settings& settings,
               QWidget* parent = nullptr);

  const TaskPtr& task() const;
  void show(const TaskPtr& task);
  using QWidget::show;
  void updateSettings();

  bool eventFilter(QObject* watched, QEvent* event) override;

private:
  void edit();
  void copyImage();
  void copyText();

  Representer& representer_;
  const Settings& settings_;
  TaskPtr task_;
  QLabel* image_;
  QLabel* recognized_;
  QLabel* separator_;
  QLabel* translated_;
  QMenu* contextMenu_;
};

#pragma once

#include <QObject>

namespace service
{
class WidgetState : public QObject
{
public:
  WidgetState(QWidget *parent = nullptr);
  void add(QWidget *watched);
  bool eventFilter(QObject *watched, QEvent *event) override;

  static void save(QWidget *widget);
  static void restore(QWidget *widget);
};

}  // namespace service

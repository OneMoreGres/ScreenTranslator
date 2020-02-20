#pragma once

#include "stfwd.h"

enum class ResultMode;
class ResultWidget;

class Representer
{
public:
  Representer(Manager &manager, TrayIcon &tray);
  ~Representer();

  void represent(const TaskPtr &task);
  void updateSettings(const Settings &settings);

private:
  void showTooltip(const TaskPtr &task);
  void showWidget(const TaskPtr &task);

  Manager &manager_;
  TrayIcon &tray_;
  std::unique_ptr<ResultWidget> widget_;
  ResultMode mode_;
};

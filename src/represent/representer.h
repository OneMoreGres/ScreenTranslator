#pragma once

#include "stfwd.h"

enum class ResultMode;
class ResultWidget;

class Representer
{
public:
  Representer(Manager &manager, TrayIcon &tray, const Settings &settings);
  ~Representer();

  void represent(const TaskPtr &task);
  void updateSettings();

private:
  void showTooltip(const TaskPtr &task);
  void showWidget(const TaskPtr &task);

  Manager &manager_;
  TrayIcon &tray_;
  const Settings &settings_;
  std::unique_ptr<ResultWidget> widget_;
};

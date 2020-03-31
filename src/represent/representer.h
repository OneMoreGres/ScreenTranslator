#pragma once

#include "stfwd.h"

enum class ResultMode;
class ResultWidget;
class ResultEditor;

class Representer
{
public:
  Representer(Manager &manager, TrayIcon &tray, const Settings &settings,
              const CommonModels &models);
  ~Representer();

  void showLast();
  void clipboardLast();
  void represent(const TaskPtr &task);
  bool isVisible() const;
  void hide();
  void updateSettings();

  void clipboardText(const TaskPtr &task);
  void clipboardImage(const TaskPtr &task);
  void edit(const TaskPtr &task);

private:
  void showTooltip(const TaskPtr &task);
  void showWidget(const TaskPtr &task);

  Manager &manager_;
  TrayIcon &tray_;
  const Settings &settings_;
  const CommonModels &models_;
  std::unique_ptr<ResultWidget> widget_;
  std::unique_ptr<ResultEditor> editor_;
};

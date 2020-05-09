#pragma once

#include "stfwd.h"

#include <QObject>

enum class ResultMode;
class ResultWidget;
class ResultEditor;

class Representer : public QObject
{
  Q_OBJECT
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

  bool eventFilter(QObject *watched, QEvent *event) override;

private:
  void showTooltip(const TaskPtr &task);
  void showWidget(const TaskPtr &task);

  Manager &manager_;
  TrayIcon &tray_;
  const Settings &settings_;
  const CommonModels &models_;
  Generation generation_{};
  std::vector<std::unique_ptr<ResultWidget>> widgets_;
  std::unique_ptr<ResultEditor> editor_;
  TaskPtr lastTooltipTask_;
};

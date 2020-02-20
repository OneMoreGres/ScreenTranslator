#include "representer.h"
#include "manager.h"
#include "resultwidget.h"
#include "settings.h"
#include "task.h"
#include "trayicon.h"

Representer::Representer(Manager &manager, TrayIcon &tray)
  : manager_(manager)
  , tray_(tray)
  , mode_{ResultMode::Widget}
{
}

Representer::~Representer() = default;

void Representer::represent(const TaskPtr &task)
{
  if (mode_ == ResultMode::Tooltip)
    showTooltip(task);
  else
    showWidget(task);
}

void Representer::updateSettings(const Settings &settings)
{
  mode_ = settings.resultShowType;
}

void Representer::showTooltip(const TaskPtr &task)
{
  auto message = task->recognized + " - " + task->translated;
  tray_.showInformation(message);
}

void Representer::showWidget(const TaskPtr &task)
{
  if (!widget_)
    widget_ = std::make_unique<ResultWidget>();

  widget_->show(task);
}

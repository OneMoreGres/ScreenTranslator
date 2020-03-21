#include "representer.h"
#include "manager.h"
#include "resultwidget.h"
#include "settings.h"
#include "task.h"
#include "trayicon.h"

Representer::Representer(Manager &manager, TrayIcon &tray,
                         const Settings &settings)
  : manager_(manager)
  , tray_(tray)
  , settings_(settings)
{
}

Representer::~Representer() = default;

void Representer::represent(const TaskPtr &task)
{
  if (settings_.resultShowType == ResultMode::Tooltip)
    showTooltip(task);
  else
    showWidget(task);
}

void Representer::updateSettings()
{
  if (widget_)
    widget_->updateSettings();
}

void Representer::showTooltip(const TaskPtr &task)
{
  auto message = task->recognized + " - " + task->translated;
  tray_.showInformation(message);
}

void Representer::showWidget(const TaskPtr &task)
{
  if (!widget_)
    widget_ = std::make_unique<ResultWidget>(settings_);

  widget_->show(task);
}

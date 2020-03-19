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
  font_ = QFont(settings.fontFamily, settings.fontSize);
  showRecognized_ = settings.showRecognized;
  showCaptured_ = settings.showCaptured;
  if (widget_)
    widget_->updateSettings(font_, showRecognized_, showCaptured_);
}

void Representer::showTooltip(const TaskPtr &task)
{
  auto message = task->recognized + " - " + task->translated;
  tray_.showInformation(message);
}

void Representer::showWidget(const TaskPtr &task)
{
  if (!widget_) {
    widget_ = std::make_unique<ResultWidget>();
    widget_->updateSettings(font_, showRecognized_, showCaptured_);
  }

  widget_->show(task);
}

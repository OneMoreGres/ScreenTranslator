#include "representer.h"
#include "debug.h"
#include "manager.h"
#include "resultwidget.h"
#include "settings.h"
#include "task.h"
#include "trayicon.h"

#include <QApplication>
#include <QClipboard>

Representer::Representer(Manager &manager, TrayIcon &tray,
                         const Settings &settings)
  : manager_(manager)
  , tray_(tray)
  , settings_(settings)
{
}

void Representer::showLast()
{
  SOFT_ASSERT(widget_, return );
  widget_->show();
}

void Representer::clipboardLast()
{
  SOFT_ASSERT(widget_, return );
  SOFT_ASSERT(widget_->task(), return );
  const auto task = widget_->task();
  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setText(task->recognized + QLatin1String(" - ") +
                     task->translated);
  tray_.showInformation(
      QObject::tr("The last result was copied to the clipboard."));
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

#include "representer.h"
#include "debug.h"
#include "geometryutils.h"
#include "manager.h"
#include "resulteditor.h"
#include "resultwidget.h"
#include "settings.h"
#include "task.h"
#include "trayicon.h"

#include <QApplication>
#include <QClipboard>
#include <QScreen>

Representer::Representer(Manager &manager, TrayIcon &tray,
                         const Settings &settings, const CommonModels &models)
  : manager_(manager)
  , tray_(tray)
  , settings_(settings)
  , models_(models)
{
}

Representer::~Representer() = default;

void Representer::showLast()
{
  SOFT_ASSERT(widget_, return );
  widget_->show();
}

void Representer::clipboardLast()
{
  SOFT_ASSERT(widget_, return );
  SOFT_ASSERT(widget_->task(), return );
  clipboardText(widget_->task());
  tray_.showInformation(
      QObject::tr("The last result was copied to the clipboard."));
}

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

void Representer::clipboardText(const TaskPtr &task)
{
  if (!task)
    return;

  QClipboard *clipboard = QApplication::clipboard();
  auto text = task->recognized;
  if (!task->translated.isEmpty())
    text += QLatin1String(" - ") + task->translated;
  clipboard->setText(text);
}

void Representer::clipboardImage(const TaskPtr &task)
{
  if (!task)
    return;

  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setPixmap(task->captured);
}

void Representer::edit(const TaskPtr &task)
{
  if (!editor_)
    editor_ = std::make_unique<ResultEditor>(manager_, models_, settings_);

  editor_->show(task);

  const auto cursor = QCursor::pos();
  const auto screen = QApplication::screenAt(cursor);
  SOFT_ASSERT(screen, return );
  editor_->move(service::geometry::cornerAtPoint(cursor, editor_->size(),
                                                 screen->geometry()));
}

void Representer::showTooltip(const TaskPtr &task)
{
  auto message = task->recognized + " - " + task->translated;
  tray_.showInformation(message);
}

void Representer::showWidget(const TaskPtr &task)
{
  if (!widget_)
    widget_ = std::make_unique<ResultWidget>(*this, settings_);

  widget_->show(task);
}

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
#include <QMouseEvent>
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
  SOFT_ASSERT(!widgets_.empty(), return );
  for (auto &widget : widgets_) {
    SOFT_ASSERT(widget->task(), continue);
    if (widget->task()->generation != generation_)
      continue;
    widget->show();
    widget->activateWindow();
  }
}

void Representer::clipboardLast()
{
  SOFT_ASSERT(!widgets_.empty(), return );
  SOFT_ASSERT(widgets_.front()->task(), return );
  clipboardText(widgets_.front()->task());
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

bool Representer::isVisible() const
{
  if (widgets_.empty())
    return false;
  return std::any_of(widgets_.cbegin(), widgets_.cend(),
                     [](const auto &w) { return w->isVisible(); });
}

void Representer::hide()
{
  if (widgets_.empty())
    return;
  for (auto &w : widgets_) w->hide();
}

void Representer::updateSettings()
{
  if (widgets_.empty())
    return;
  for (auto &w : widgets_) w->updateSettings();
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

bool Representer::eventFilter(QObject * /*watched*/, QEvent *event)
{
  if (event->type() == QEvent::WindowDeactivate) {
    for (const auto &w : widgets_) {
      if (w->isActiveWindow())
        return false;
    }
    hide();
  } else if (event->type() == QEvent::MouseButtonPress) {
    const auto casted = static_cast<QMouseEvent *>(event);
    if (casted->button() == Qt::LeftButton)
      hide();
  }
  return false;
}

void Representer::showTooltip(const TaskPtr &task)
{
  auto message = task->recognized + " - " + task->translated;
  tray_.showInformation(message);
}

void Representer::showWidget(const TaskPtr &task)
{
  generation_ = task->generation;

  auto index = 0u;
  const auto count = widgets_.size();
  for (; index < count; ++index) {
    auto &widget = widgets_[index];
    SOFT_ASSERT(widget->task(), continue);
    if (widget->task()->generation != generation_)
      break;
  }

  if (index == count) {
    widgets_.emplace_back(std::make_unique<ResultWidget>(*this, settings_));
    widgets_.back()->installEventFilter(this);
  }

  auto &widget = widgets_[index];
  widget->show(task);
}

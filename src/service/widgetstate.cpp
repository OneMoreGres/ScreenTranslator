#include "widgetstate.h"
#include "debug.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QHeaderView>
#include <QMainWindow>
#include <QSettings>
#include <QSplitter>
#include <QTableView>

namespace service
{
namespace
{
enum class Action { Save, Restore };

void handleGeometry(QSettings *settings, QWidget *widget, Action action)
{
  SOFT_ASSERT(widget, return );
  if (widget->parent())
    return;

  const auto name = widget->objectName() + QLatin1String("_geometry");
  if (action == Action::Save) {
    settings->setValue(name, widget->geometry());
  } else {
    if (settings->contains(name))
      widget->setGeometry(settings->value(name).toRect());
  }
}

template <class T>
void handleState(QSettings *settings, QWidget *widget, Action action)
{
  auto instance = qobject_cast<T *>(widget);
  if (!instance)
    return;

  const auto name = widget->objectName() + QLatin1Char('_') + typeid(T).name();
  if (action == Action::Save)
    settings->setValue(name, instance->saveState());
  else
    instance->restoreState(settings->value(name).toByteArray());
}

void handleWidget(QSettings *settings, QWidget *widget, Action action)
{
  SOFT_ASSERT(widget, return );
  if (!widget->objectName().isEmpty()) {
    handleGeometry(settings, widget, action);
    handleState<QSplitter>(settings, widget, action);
    handleState<QHeaderView>(settings, widget, action);
    handleState<QMainWindow>(settings, widget, action);
  }

  settings->beginGroup(widget->objectName());
  const auto children =
      widget->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly);
  for (auto *child : children) {
    handleWidget(settings, child, action);
  }
  settings->endGroup();
}

void apply(QWidget *widget, Action action)
{
  QSettings settings;
  settings.beginGroup(QStringLiteral("GUI"));

  handleWidget(&settings, widget, action);

  settings.endGroup();
}
}  // namespace

WidgetState::WidgetState(QWidget *parent)
  : QObject(parent)
{
  add(parent);
}

void WidgetState::add(QWidget *watched)
{
  if (!watched)
    return;
  watched->installEventFilter(this);
}

bool WidgetState::eventFilter(QObject *watched, QEvent *event)
{
  if (event->type() != QEvent::Show && event->type() != QEvent::Hide)
    return QObject::eventFilter(watched, event);

  auto widget = qobject_cast<QWidget *>(watched);
  if (!widget)
    return QObject::eventFilter(watched, event);

  if (event->type() == QEvent::Show)
    restore(widget);
  else if (event->type() == QEvent::Hide)
    save(widget);

  return QObject::eventFilter(watched, event);
}

void WidgetState::addHelp(QCommandLineParser &parser)
{
  parser.addOption(
      {"reset-gui", QObject::tr("Do not restore user interface "
                                "(window size and position, etc)")});
}

void WidgetState::save(QWidget *widget)
{
  SOFT_ASSERT(widget, return );
  SOFT_ASSERT(!widget->objectName().isEmpty(), return );
  apply(widget, Action::Save);
}

void WidgetState::restore(QWidget *widget)
{
  if (QCoreApplication::arguments().contains(QLatin1String("--reset-gui")))
    return;
  SOFT_ASSERT(widget, return );
  SOFT_ASSERT(!widget->objectName().isEmpty(), return );
  apply(widget, Action::Restore);
}

}  // namespace service

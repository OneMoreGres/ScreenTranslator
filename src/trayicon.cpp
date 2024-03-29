#include "trayicon.h"
#include "debug.h"
#include "globalaction.h"
#include "manager.h"
#include "settings.h"

#include <QMenu>
#include <QTimer>

using GlobalAction = service::GlobalAction;

TrayIcon::TrayIcon(Manager &manager, const Settings &settings)
  : manager_(manager)
  , settings_(settings)
  , tray_(std::make_unique<QSystemTrayIcon>())
  , iconUpdateTimer_(std::make_unique<QTimer>())
{
  GlobalAction::init();

  connect(tray_.get(), &QSystemTrayIcon::activated,  //
          this, &TrayIcon::handleIconClick);

  iconUpdateTimer_->setSingleShot(true);
  connect(iconUpdateTimer_.get(), &QTimer::timeout,  //
          this, &TrayIcon::updateIcon);

  tray_->setContextMenu(contextMenu());
  setIcon(Icon::Idle, Duration::Permanent);
  updateActions();
  tray_->show();
}

TrayIcon::~TrayIcon() = default;

void TrayIcon::updateSettings()
{
  QStringList failedActions;
  if (!GlobalAction::update(captureAction_, settings_.captureHotkey))
    failedActions << settings_.captureHotkey;
  if (!GlobalAction::update(repeatCaptureAction_,
                            settings_.repeatCaptureHotkey))
    failedActions << settings_.repeatCaptureHotkey;
  if (!GlobalAction::update(showLastAction_, settings_.showLastHotkey))
    failedActions << settings_.showLastHotkey;
  if (!GlobalAction::update(clipboardAction_, settings_.clipboardHotkey))
    failedActions << settings_.clipboardHotkey;
  if (!GlobalAction::update(captureLockedAction_,
                            settings_.captureLockedHotkey))
    failedActions << settings_.captureLockedHotkey;

  if (!failedActions.isEmpty()) {
    showError(tr("Failed to register global shortcuts:\n%1"
                 "\nMost likely they are already in use by another program")
                  .arg(failedActions.join('\n')));
  }
}

void TrayIcon::blockActions(bool block)
{
  isActionsBlocked_ = block;
  updateActions();
  const auto actions =
      QVector<QAction *>{captureAction_, repeatCaptureAction_, showLastAction_,
                         clipboardAction_, captureLockedAction_};
  for (const auto i : actions) {
    if (block) {
      GlobalAction::removeGlobal(i);
    } else {
      GlobalAction::makeGlobal(i);
    }
  }
}

void TrayIcon::setTaskActionsEnabled(bool isEnabled)
{
  gotTask_ = isEnabled;
  updateActions();
}

void TrayIcon::setCaptureLockedEnabled(bool isEnabled)
{
  canCaptureLocked_ = isEnabled;
  updateActions();
}

void TrayIcon::setRepeatCaptureEnabled(bool isEnabled)
{
  canRepeatCapture_ = isEnabled;
  updateActions();
}

void TrayIcon::updateActions()
{
  if (isActionsBlocked_) {
    QVector<QAction *> blockable{captureAction_, repeatCaptureAction_,
                                 showLastAction_, settingsAction_,
                                 captureLockedAction_};
    for (auto &action : blockable) action->setEnabled(false);
    return;
  }

  captureAction_->setEnabled(true);
  settingsAction_->setEnabled(true);

  QVector<QAction *> taskActions{showLastAction_, clipboardAction_};
  for (auto &action : taskActions) action->setEnabled(gotTask_);

  repeatCaptureAction_->setEnabled(canRepeatCapture_);
  captureLockedAction_->setEnabled(canCaptureLocked_);
}

void TrayIcon::setIcon(TrayIcon::Icon icon, Duration duration)
{
  QMap<Icon, QString> icons{
      {Icon::Idle, QStringLiteral(":icons/app.png")},
      {Icon::Success, QStringLiteral(":icons/st_success.png")},
      {Icon::Busy, QStringLiteral(":icons/st_busy.png")},
      {Icon::Error, QStringLiteral(":icons/st_error.png")},
  };

  tray_->setIcon(QIcon(icons.value(icon)));

  if (duration == Duration::Permanent) {
    permanentIcon_ = icon;
    return;
  }

  const auto durationMsec = 3000;
  iconUpdateTimer_->start(durationMsec);
}

void TrayIcon::setActiveTaskCount(int count)
{
  activeTaskCount_ = count;
  updateIcon();
}

void TrayIcon::resetFatalError()
{
  isFatalError_ = false;
  updateIcon();
}

void TrayIcon::updateIcon()
{
  if (iconUpdateTimer_->isActive())
    return;

  if (isFatalError_) {
    setIcon(Icon::Error, Duration::Permanent);
    return;
  }

  setIcon(activeTaskCount_ > 0 ? Icon::Busy : Icon::Idle, Duration::Permanent);
}

void TrayIcon::showInformation(const QString &text)
{
  tray_->showMessage({}, text, QSystemTrayIcon::Information);
}

void TrayIcon::showError(const QString &text)
{
  LERROR() << text;
  setIcon(Icon::Error, Duration::Temporal);
  tray_->showMessage(tr("Error"), text, QSystemTrayIcon::Warning);
}

void TrayIcon::showFatalError(const QString &text)
{
  LERROR() << text;
  isFatalError_ = true;
  tray_->showMessage(tr("Error"), text, QSystemTrayIcon::Critical);
  updateIcon();
}

void TrayIcon::showSuccess()
{
  setIcon(Icon::Success, Duration::Temporal);
}

void TrayIcon::handleIconClick(QSystemTrayIcon::ActivationReason reason)
{
  if (reason == QSystemTrayIcon::Trigger && showLastAction_->isEnabled()) {
    manager_.showLast();
    return;
  }

  if (reason == QSystemTrayIcon::MiddleClick && clipboardAction_->isEnabled()) {
    manager_.copyLastToClipboard();
    return;
  }

  if (reason == QSystemTrayIcon::DoubleClick &&
      repeatCaptureAction_->isEnabled()) {
    manager_.repeatCapture();
  }
}

QMenu *TrayIcon::contextMenu()
{
  QMenu *menu = new QMenu();
  {
    captureAction_ = menu->addAction(tr("Capture"));
    connect(captureAction_, &QAction::triggered,  //
            this, [this] { manager_.capture(); });
  }
  {
    repeatCaptureAction_ = menu->addAction(tr("Repeat capture"));
    connect(repeatCaptureAction_, &QAction::triggered,  //
            this, [this] { manager_.repeatCapture(); });
  }
  {
    captureLockedAction_ = menu->addAction(tr("Capture saved areas"));
    connect(captureLockedAction_, &QAction::triggered,  //
            this, [this] { manager_.captureLocked(); });
  }

  {
    QMenu *translateMenu = menu->addMenu(tr("Result"));
    {
      showLastAction_ = translateMenu->addAction(tr("Show"));
      connect(showLastAction_, &QAction::triggered,  //
              this, [this] { manager_.showLast(); });
    }
    {
      clipboardAction_ = translateMenu->addAction(tr("To clipboard"));
      connect(clipboardAction_, &QAction::triggered,  //
              this, [this] { manager_.copyLastToClipboard(); });
    }
  }

  {
    auto action = menu->addAction(tr("Show translator"));
    connect(action, &QAction::triggered,  //
            this, [this] { manager_.showTranslator(); });
  }

  {
    settingsAction_ = menu->addAction(tr("Settings"));
    connect(settingsAction_, &QAction::triggered,  //
            this, [this] { manager_.settings(); });
  }

  {
    auto action = menu->addAction(tr("Quit"));
    connect(action, &QAction::triggered,  //
            this, [this] { manager_.quit(); });
  }

  return menu;
}

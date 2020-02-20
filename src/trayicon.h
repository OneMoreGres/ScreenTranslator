#pragma once

#include "stfwd.h"

#include <QSystemTrayIcon>

class QAction;

class TrayIcon : public QObject
{
  Q_OBJECT
public:
  explicit TrayIcon(Manager &manager);
  ~TrayIcon();

  void updateSettings(const Settings &settings);

  void blockActions(bool block);
  void setTaskActionsEnabled(bool isEnabled);
  void setRepeatCaptureEnabled(bool isEnabled);
  void setActiveTaskCount(int count);
  void resetFatalError();

  void showInformation(const QString &text);
  void showError(const QString &text);
  void showFatalError(const QString &text);
  void showSuccess();

private:
  enum class Icon { Idle, Success, Busy, Error };
  enum Duration { Permanent, Temporal };
  void setIcon(TrayIcon::Icon icon, Duration duration);
  void handleIconClick(QSystemTrayIcon::ActivationReason reason);
  QMenu *contextMenu();
  void updateIcon();
  void updateActions();

  Manager &manager_;
  std::unique_ptr<QSystemTrayIcon> tray_;

  QAction *captureAction_{nullptr};
  QAction *repeatCaptureAction_{nullptr};
  QAction *showLastAction_{nullptr};
  QAction *clipboardAction_{nullptr};
  QAction *settingsAction_{nullptr};

  std::unique_ptr<QTimer> iconUpdateTimer_;
  int activeTaskCount_{0};
  bool isFatalError_{false};
  Icon permanentIcon_{Icon::Idle};

  bool gotTask_{false};
  bool canRepeatCapture_{false};
  bool isActionsBlocked_{false};
};

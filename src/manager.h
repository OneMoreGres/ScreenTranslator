#pragma once

#include "stfwd.h"

class QString;

class Manager
{
public:
  Manager();
  ~Manager();

  void captured(const TaskPtr &task);
  void captureCanceled();
  void recognized(const TaskPtr &task);
  void corrected(const TaskPtr &task);
  void translated(const TaskPtr &task);

  void applySettings(const Settings &settings);
  void fatalError(const QString &text);
  void capture();
  void repeatCapture();
  void showLast();
  void settings();
  void copyLastToClipboard();
  void about();
  void quit();

private:
  void updateSettings();
  void setupProxy(const Settings &settings);
  void finishTask(const TaskPtr &task);

  std::unique_ptr<Settings> settings_;
  std::unique_ptr<TrayIcon> tray_;
  std::unique_ptr<Capturer> capturer_;
  std::unique_ptr<Recognizer> recognizer_;
  std::unique_ptr<Corrector> corrector_;
  std::unique_ptr<Translator> translator_;
  std::unique_ptr<Representer> representer_;
  std::unique_ptr<update::Loader> updater_;
  std::unique_ptr<update::AutoChecker> updateAutoChecker_;
  int activeTaskCount_{0};
};

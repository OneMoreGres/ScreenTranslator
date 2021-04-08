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
  void captureLocked();
  void showLast();
  void showTranslator();
  void settings();
  void copyLastToClipboard();
  void quit();

private:
  void updateSettings();
  void setupProxy(const Settings &settings);
  void setupUpdates(const Settings &settings);
  bool setupTrace(bool isOn);
  void finishTask(const TaskPtr &task);
  void warnIfOutdated();

  std::unique_ptr<CommonModels> models_;
  std::unique_ptr<Settings> settings_;
  std::unique_ptr<TrayIcon> tray_;
  std::unique_ptr<Capturer> capturer_;
  std::unique_ptr<Recognizer> recognizer_;
  std::unique_ptr<Corrector> corrector_;
  std::unique_ptr<Translator> translator_;
  std::unique_ptr<Representer> representer_;
  std::unique_ptr<update::Updater> updater_;
  int activeTaskCount_{0};
};

#pragma once

#include "stfwd.h"

#include <QWidget>

class QWebEngineView;
class QTabWidget;

class WebPage;

class Translator : public QWidget
{
  Q_OBJECT
public:
  explicit Translator(Manager &manager);
  ~Translator();

  void translate(const TaskPtr &task);
  void updateSettings(const Settings &settings);
  void finish(const TaskPtr &task);

protected:
  void timerEvent(QTimerEvent *event) override;
  void closeEvent(QCloseEvent *event) override;

private:
  void changeCurrentPage(int tabIndex);
  void processQueue();
  void markTranslated(const TaskPtr &task);

  Manager &manager_;
  QWebEngineView *view_;
  QTabWidget *tabs_;
  std::vector<TaskPtr> queue_;
  std::map<QString, std::unique_ptr<WebPage>> pages_;
};

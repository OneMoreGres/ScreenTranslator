#pragma once

#include <QDialog>

#include "commonmodels.h"
#include "settings.h"

namespace Ui
{
class SettingsEditor;
}
class QAbstractButton;

class SettingsEditor : public QDialog
{
  Q_OBJECT

public:
  SettingsEditor(Manager &manager, update::Loader &updater);
  ~SettingsEditor();

  Settings settings() const;
  void setSettings(const Settings &settings);

private:
  void updateCurrentPage();
  void updateTranslators();
  void adjustUpdatesView();
  void handleButtonBoxClicked(QAbstractButton *button);
  void handlePortableChanged();
  void updateResultFont();
  void updateModels(const QString &tessdataPath);

  Ui::SettingsEditor *ui;
  Manager &manager_;
  update::Loader &updater_;
  CommonModels models_;
  QStringList enabledTranslators_;
  bool wasPortable_{false};
};

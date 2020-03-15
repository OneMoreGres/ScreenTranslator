#pragma once

#include <QDialog>

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
  void updateTesseractLanguages();
  void updateTranslators();
  void updateTranslationLanguages();
  void adjustUpdatesView();
  void handleButtonBoxClicked(QAbstractButton *button);

  Ui::SettingsEditor *ui;
  Manager &manager_;
  update::Loader &updater_;
  QStringList enabledTranslators_;
};

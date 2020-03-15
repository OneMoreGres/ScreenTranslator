#pragma once

#include <QDialog>

#include "settings.h"

namespace Ui
{
class SettingsEditor;
}

class SettingsEditor : public QDialog
{
  Q_OBJECT

public:
  explicit SettingsEditor(update::Loader &updater);
  ~SettingsEditor();

  Settings settings() const;
  void setSettings(const Settings &settings);

private:
  void updateCurrentPage();
  void updateTesseractLanguages();
  void updateTranslators();
  void updateTranslationLanguages();
  void adjustUpdatesView();

  Ui::SettingsEditor *ui;
  update::Loader &updater_;
  QStringList enabledTranslators_;
};

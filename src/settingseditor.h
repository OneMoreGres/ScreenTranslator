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
  SettingsEditor(Manager &manager, update::Updater &updater);
  ~SettingsEditor();

  Settings settings() const;
  void setSettings(const Settings &settings);

private:
  enum ColorContext { Font, Bagkround };
  void updateCurrentPage();
  void updateTranslators();
  void adjustUpdatesView();
  void handleButtonBoxClicked(QAbstractButton *button);
  void handlePortableChanged();
  void updateResultFont();
  void updateModels(const QString &tessdataPath);
  void pickColor(ColorContext context);

  Ui::SettingsEditor *ui;
  Manager &manager_;
  update::Updater &updater_;
  CommonModels models_;
  QStringList enabledTranslators_;
  bool wasPortable_{false};
};

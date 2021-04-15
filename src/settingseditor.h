#pragma once

#include <QDialog>

#include "commonmodels.h"
#include "settings.h"

namespace Ui
{
class SettingsEditor;
}
class QAbstractButton;
class QStandardItemModel;

class SettingsEditor : public QDialog
{
  Q_OBJECT

public:
  SettingsEditor(Manager &manager, update::Updater &updater);
  ~SettingsEditor();

  Settings settings() const;
  void setSettings(const Settings &settings);

private:
  enum ColorContext { Font, Background };
  void handleButtonBoxClicked(QAbstractButton *button);
  void pickColor(ColorContext context);
  void updateResultFont();
  QStringList enabledTranslators() const;

  void updateState();
  void updateCurrentPage();
  void updateTranslators(const QStringList &translators);
  void updateModels();
  void validateSettings();

  Ui::SettingsEditor *ui;
  Manager &manager_;
  update::Updater &updater_;
  CommonModels models_;
  bool wasPortable_{false};
  QStandardItemModel *pageModel_;
};

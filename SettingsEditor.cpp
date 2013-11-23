#include "SettingsEditor.h"
#include "ui_SettingsEditor.h"

SettingsEditor::SettingsEditor(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SettingsEditor)
{
  ui->setupUi(this);
}

SettingsEditor::~SettingsEditor()
{
  delete ui;
}

#ifndef SETTINGSEDITOR_H
#define SETTINGSEDITOR_H

#include <QDialog>

namespace Ui {
  class SettingsEditor;
}

class SettingsEditor : public QDialog
{
    Q_OBJECT

  public:
    explicit SettingsEditor(QWidget *parent = 0);
    ~SettingsEditor();

  signals:
    void settingsEdited ();

  public slots:
    void done (int result);

  private slots:
    void saveSettings () const;
    void openTessdataDialog ();
    void initOcrLangCombo ();

  private:
    void loadSettings ();
    void saveState () const;
    void loadState ();
    void initTranslateLangCombo ();

  private:
    Ui::SettingsEditor *ui;
};

#endif // SETTINGSEDITOR_H

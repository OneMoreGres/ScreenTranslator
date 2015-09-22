#ifndef SETTINGSEDITOR_H
#define SETTINGSEDITOR_H

#include <QDialog>
#include <QButtonGroup>
#include <QMap>

namespace Ui {
  class SettingsEditor;
}
class LanguageHelper;

class SettingsEditor : public QDialog {
  Q_OBJECT

  public:
    explicit SettingsEditor (const LanguageHelper &dictionary, QWidget *parent = 0);
    ~SettingsEditor ();

  signals:
    void settingsEdited ();

  public slots:
    void done (int result);

  private slots:
    void saveSettings () const;
    void openTessdataDialog ();
    void initOcrLangCombo (const QString &path);

  private:
    void loadSettings ();
    void saveState () const;
    void loadState ();

  private:
    Ui::SettingsEditor *ui;
    const LanguageHelper &dictionary_;
    QButtonGroup *buttonGroup_;
};

#endif // SETTINGSEDITOR_H

#ifndef SETTINGSEDITOR_H
#define SETTINGSEDITOR_H

#include <QDialog>
#include <QButtonGroup>
#include <QMap>

class QTableWidgetItem;
namespace Ui {
  class SettingsEditor;
}
class LanguageHelper;
class TranslatorHelper;
class RecognizerHelper;

class SettingsEditor : public QDialog {
  Q_OBJECT

  enum SubsCol {
    SubsColLanguage = 0, SubsColSource, SubsColTarget
  };

  public:
    explicit SettingsEditor (const LanguageHelper &dictionary, QWidget *parent = 0);
    ~SettingsEditor ();

  signals:
    void settingsEdited ();
    void updateCheckRequested ();

  public slots:
    void done (int result);

  private slots:
    void saveSettings () const;
    void openTessdataDialog ();
    void initOcrLangCombo (const QString &path);
    void recognizerFixTableItemChanged (QTableWidgetItem *item);

  private:
    void loadSettings ();
    void saveState () const;
    void loadState ();
    bool initSubsTableRow (int row, const QString &languageCode = QString ());

  private:
    Ui::SettingsEditor *ui;
    TranslatorHelper *translatorHelper_;
    RecognizerHelper *recognizerHelper_;
    const LanguageHelper &dictionary_;
    QButtonGroup *buttonGroup_;
};

#endif // SETTINGSEDITOR_H

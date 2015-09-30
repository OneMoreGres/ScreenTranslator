#ifndef LANGUAGEHELPER_H
#define LANGUAGEHELPER_H

#include <QMap>
#include <QStringList>
#include <QMenu>

class LanguageHelper {
  public:
    LanguageHelper ();

    QStringList availableOcrLanguagesUi () const;
    const QStringList &availableOcrLanguages () const;
    QStringList availableOcrLanguagesUi (const QString &path) const;
    QStringList translateLanguagesUi () const;
    QStringList translateLanguages () const;

    QString translateCodeToUi (const QString &text) const;
    QString translateUiToCode (const QString &text) const;
    QString ocrCodeToUi (const QString &text) const;
    QString ocrUiToCode (const QString &text) const;
    QString translateForOcrCode (const QString &text) const;

    void updateAvailableOcrLanguages ();

    //! Update languages menu. Group items into submenus if needed.
    void updateMenu (QMenu *menu, const QStringList &languages, int groupSize = 10) const;

  private:
    QStringList availableOcrLanguages (const QString &path) const;
    void init ();
    void initTranslateLanguages ();
    void initOcrLanguages ();

  private:
    QStringList availableOcrLanguages_;
    QMap<QString, QString> translateLanguages_;
    QMap<QString, QString> ocrLanguages_;
};


#endif // LANGUAGEHELPER_H

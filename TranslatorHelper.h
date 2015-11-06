#ifndef TRANSLATORHELPER_H
#define TRANSLATORHELPER_H

#include <QStringList>

class TranslatorHelper {
  public:
    TranslatorHelper ();

    QStringList possibleTranslators (QStringList &enabled) const;
    QStringList enabledTranslatorScripts () const;

    void setEnabledTranslators (const QStringList &enabled) const;

    void loadScripts ();
    void newItem ();
    QString currentScript () const;
    QString nextScript ();
    bool gotScripts () const;

  private:
    QString translatorsDir_;
    QStringList scripts_;
    int currentIndex_;
};

#endif // TRANSLATORHELPER_H

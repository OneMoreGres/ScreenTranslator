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
    void newItem (bool forceRotate);
    QString nextScript ();
    QString currentScript () const;
    bool gotScripts () const;

  private:
    QString translatorsDir_;
    QStringList scripts_;
    int currentIndex_;
    int triesLeft_;
};

#endif // TRANSLATORHELPER_H

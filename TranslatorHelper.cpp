#include <QSettings>
#include <QDir>
#include <QFile>
#include <QApplication>

#include "TranslatorHelper.h"
#include "Settings.h"

TranslatorHelper::TranslatorHelper ()
  : translatorsDir_ ("translators"), currentIndex_ (0), triesLeft_ (0) {
  translatorsDir_ = QApplication::applicationDirPath () + QDir::separator () + translatorsDir_;
}

void TranslatorHelper::setEnabledTranslators (const QStringList &enabled) const {
  QSettings settings;
  settings.beginGroup (settings_names::translationGroup);
  settings.setValue (settings_names::translators, enabled.join ("|"));
}

QStringList TranslatorHelper::possibleTranslators (QStringList &enabled) const {
#define GET(FIELD) settings.value (settings_names::FIELD, settings_values::FIELD)
  QSettings settings;
  settings.beginGroup (settings_names::translationGroup);
  QStringList exist = QDir (translatorsDir_).entryList (QStringList () << "*.js", QDir::Files);
  QStringList saved = GET (translators).toString ().split ("|", QString::SkipEmptyParts);
  QStringList on, off;
  std::copy_if (saved.begin (), saved.end (), std::back_inserter (on), [&](const QString &i) {
    return exist.contains (i);
  });
  on = on.isEmpty () ? exist : on;

  std::copy_if (exist.begin (), exist.end (), std::back_inserter (off), [&](const QString &i) {
    return !on.contains (i);
  });

  enabled = on;
  return (on + off);
#undef GET
}

QStringList TranslatorHelper::enabledTranslatorScripts () const {
  QStringList enabled;
  possibleTranslators (enabled);
  QStringList scripts;
  foreach (const QString &name, enabled) {
    QFile f (translatorsDir_ + QDir::separator () + name);
    if (f.open (QFile::ReadOnly)) {
      QString script = QString::fromUtf8 (f.readAll ());
      if (!script.isEmpty ()) {
        scripts << script;
      }
    }
  }
  return scripts;
}

void TranslatorHelper::loadScripts () {
  scripts_ = enabledTranslatorScripts ();
}

void TranslatorHelper::newItem (bool forceRotate) {
  triesLeft_ = scripts_.size ();
  currentIndex_ = forceRotate ? currentIndex_ + 1 : 0;
  if (currentIndex_ >= scripts_.size ()) {
    currentIndex_ = 0;
  }
}

QString TranslatorHelper::nextScript () {
  --triesLeft_;

  if (++currentIndex_ >= scripts_.size ()) {
    currentIndex_ = 0;
  }

  return currentScript ();
}

QString TranslatorHelper::currentScript () const {
  if (triesLeft_ > 0 && currentIndex_ < scripts_.size ()) {
    return scripts_.at (currentIndex_);
  }
  return QString ();
}

bool TranslatorHelper::gotScripts () const {
  return !scripts_.isEmpty ();
}

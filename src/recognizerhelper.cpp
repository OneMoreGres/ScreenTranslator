#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QApplication>

#include "recognizerhelper.h"

RecognizerHelper::RecognizerHelper ()
  : fileName_ ("st_subs.csv") {
#ifdef Q_OS_LINUX
  QDir settingDir = QFileInfo (QSettings ().fileName ()).dir ();
  fileName_ = settingDir.absoluteFilePath (fileName_);
#else
  fileName_ = QApplication::applicationDirPath () + QDir::separator () + fileName_;
#endif
}

void RecognizerHelper::load () {
  subs_.clear ();
  QFile f (fileName_);
  if (!f.open (QFile::ReadOnly)) {
    return;
  }
  QByteArray data = f.readAll ();
  f.close ();
  QStringList lines = QString::fromUtf8 (data).split ('\n', QString::SkipEmptyParts);
  foreach (const QString &line, lines) {
    QStringList parts = line.mid (1, line.size () - 2).split ("\",\""); // remove "
    if (parts.size () < 3) {
      continue;
    }
    subs_.append (Sub (parts[0], parts[1], parts[2]));
  }
}

void RecognizerHelper::save () {
  QFile f (fileName_);
  if (!f.open (QFile::WriteOnly)) {
    return;
  }
  foreach (const Sub &sub, subs_) {
    QStringList parts = QStringList () << sub.language << sub.source << sub.target;
    QString line = "\"" + parts.join ("\",\"") + "\"\n";
    f.write (line.toUtf8 ());
  }
  f.close ();
}

QString RecognizerHelper::substitute (const QString &source, const QString &language) const {
  QString result = source;
  while (true) {
    int bestMatchIndex = -1;
    int bestMatchLen = 0;
    int index = -1;
    foreach (const Sub &sub, subs_) {
      ++index;
      if (sub.language != language || !result.contains (sub.source)) {
        continue;
      }
      int len = sub.source.length ();
      if (len > bestMatchLen) {
        bestMatchLen = len;
        bestMatchIndex = index;
      }
    }
    if (bestMatchIndex > -1) {
      const Sub &sub = subs_.at (bestMatchIndex);
      result.replace (sub.source, sub.target);
      continue;
    }
    break;
  }

  return result;
}

const RecognizerHelper::Subs &RecognizerHelper::subs () const {
  return subs_;
}

void RecognizerHelper::setSubs (const Subs &subs) {
  subs_ = subs;
}

RecognizerHelper::Sub::Sub (const QString &language, const QString &source, const QString &target)
  : language (language), source (source), target (target) {
}

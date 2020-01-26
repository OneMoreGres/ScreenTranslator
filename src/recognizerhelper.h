#ifndef RECOGNIZERHELPER_H
#define RECOGNIZERHELPER_H

#include <QString>

class RecognizerHelper {
  public:
    struct Sub {
      Sub (const QString &language = QString (), const QString &source = QString (),
           const QString &target = QString ());
      QString language;
      QString source;
      QString target;
    };
    typedef QList<Sub> Subs;

  public:
    RecognizerHelper ();

    void load ();
    void save ();

    QString substitute (const QString &source, const QString& language) const;

    const Subs &subs () const;
    void setSubs (const Subs &subs);

  private:
    QString fileName_;
    Subs subs_;
};

#endif // RECOGNIZERHELPER_H

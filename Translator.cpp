#include "Translator.h"

#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QSettings>

#include "Settings.h"

namespace
{
  const QString translateBaseUrl = "http://translate.google.com/translate_a/"
                                   "t?client=t&text=%1&sl=auto&tl=%2";
}

Translator::Translator(QObject *parent) :
  QObject(parent),
  network_ (this)
{
  connect (&network_, SIGNAL (finished (QNetworkReply*)),
           SLOT (replyFinished (QNetworkReply*)));

  applySettings ();
}

void Translator::applySettings()
{
  QSettings settings;
  settings.beginGroup (settings_names::translationGroup);
  translationLanguage_ = settings.value (settings_names::translationLanguage,
                                         settings_values::translationLanguage).
                         toString ();
}

void Translator::translate(QString text)
{
  Q_ASSERT (!text.isEmpty ());
  if (translationLanguage_.isEmpty ())
  {
    emit error (tr ("Неверные парметры для перевода."));
    return;
  }
  QUrl url (translateBaseUrl.arg (text, translationLanguage_));
  network_.get (QNetworkRequest (url));
}

void Translator::replyFinished(QNetworkReply* reply)
{
  Q_ASSERT (reply->isFinished ());
  if (reply->error () != QNetworkReply::NoError)
  {
    emit error (tr ("Ошибка перевода: %1").arg (reply->errorString ()));
    reply->deleteLater ();
    return;
  }
  QByteArray data = reply->readAll ();
  reply->deleteLater ();

  while (data.indexOf (",,") != -1)//make json valid
  {
    data.replace (",,", ",");
  }
  QJsonParseError parseError;
  QJsonDocument document = QJsonDocument::fromJson (data, &parseError);
  if (document.isEmpty ())
  {
    emit error (tr ("Ошибка разбора перевода: %1 (%2)").
                arg (parseError.errorString ()).arg (parseError.offset));
    return;
  }
  QJsonArray answerArray = document.array ();
  QJsonArray fullTranslation = answerArray.first ().toArray ();
  QString source = "";
  QString translation = "";
  foreach (QJsonValue part, fullTranslation)
  {
    QJsonArray partTranslation = part.toArray ();
    if (partTranslation.isEmpty ())
    {
      continue;
    }
    translation += partTranslation.at (0).toString ();
    source += partTranslation.at (1).toString ();
  }
  emit translated (source, translation);
}

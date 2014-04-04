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
                                   "t?client=t&text=%1&sl=%2&tl=%3";
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
                                         settings_values::translationLanguage).toString ();
  sourceLanguage_ = settings.value (settings_names::sourceLanguage,
                                    settings_values::sourceLanguage).toString ();
}

void Translator::translate(ProcessingItem item)
{
  Q_ASSERT (!item.recognized.isEmpty ());
  QString sourceLanguage = item.sourceLanguage.isEmpty () ? sourceLanguage_ :
                                                            item.sourceLanguage;
  if (translationLanguage_.isEmpty () || sourceLanguage.isEmpty ())
  {
    emit error (tr ("Неверные парметры для перевода."));
    return;
  }
  QUrl url (translateBaseUrl.arg (item.recognized, sourceLanguage, translationLanguage_));
  QNetworkReply* reply = network_.get (QNetworkRequest (url));
  items_.insert (reply, item);
}

void Translator::replyFinished(QNetworkReply* reply)
{
  Q_ASSERT (items_.contains (reply));
  ProcessingItem item = items_.take (reply);
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
  QString translation = "";
  foreach (QJsonValue part, fullTranslation)
  {
    QJsonArray partTranslation = part.toArray ();
    if (partTranslation.isEmpty ())
    {
      continue;
    }
    translation += partTranslation.at (0).toString ();
  }
  item.translated = translation;
  emit translated (item);
}

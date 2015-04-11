#include "Translator.h"

#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QSettings>

#include "Settings.h"
#include "GoogleWebTranslator.h"

namespace
{
  const QString translateBaseUrl = "http://translate.google.com/translate_a/"
                                   "t?client=t&text=%1&sl=%2&tl=%3";
}

Translator::Translator(QObject *parent) :
  QObject(parent),
  network_ (this),
  useAlternativeTranslation_ (false)
{
  connect (&network_, SIGNAL (finished (QNetworkReply*)),
           SLOT (replyFinished (QNetworkReply*)));

  GoogleWebTranslator* googleWeb = new GoogleWebTranslator;
  connect (this, SIGNAL (translateAlternative (ProcessingItem)),
           googleWeb, SLOT (translate (ProcessingItem)));
  connect (googleWeb, SIGNAL (translated (ProcessingItem, bool)),
           this, SLOT (translatedAlternative(ProcessingItem, bool)));
  connect (googleWeb, SIGNAL (error (QString)), this, SIGNAL (error (QString)));

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
  if (useAlternativeTranslation_) {
    emit translateAlternative(item);
    return;
  }
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

void Translator::translatedAlternative(ProcessingItem item, bool success)
{
  if (success)
  {
    emit translated(item);
  }
  else
  {
    emit error (tr ("Ошибка альтернативного перевода текста: %1").arg (item.recognized));
  }
}

void Translator::replyFinished(QNetworkReply* reply)
{
  Q_ASSERT (items_.contains (reply));
  ProcessingItem item = items_.take (reply);
  Q_ASSERT (reply->isFinished ());
  if (reply->error () != QNetworkReply::NoError)
  {
    useAlternativeTranslation_ = true;
    emit translateAlternative(item);
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
    useAlternativeTranslation_ = true;
    emit translateAlternative(item);
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

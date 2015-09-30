#include <QWebView>
#include <QWebFrame>
#include <QWebElement>
#include <QSettings>
#include <QNetworkReply>
#include <QTimer>

#include "GoogleWebTranslator.h"
#include "Settings.h"
#include "StAssert.h"

GoogleWebTranslator::GoogleWebTranslator ()
  : QObject (), view_ (new QWebView),
  isLoadFinished_ (true), isTranslationFinished_ (false) {

  view_->settings ()->setAttribute (QWebSettings::AutoLoadImages, false);
  connect (view_, SIGNAL (loadStarted ()), this, SLOT (loadStarted ()));
  connect (view_, SIGNAL (loadFinished (bool)), this, SLOT (loadFinished (bool)));

  connect (view_->page ()->networkAccessManager (), SIGNAL (finished (QNetworkReply *)),
           this, SLOT (replyFinished (QNetworkReply *)));
  applySettings ();
}

GoogleWebTranslator::~GoogleWebTranslator () {
  delete view_;
}

void GoogleWebTranslator::translate (ProcessingItem item) {
  queue_.push_back (item);
  if (isLoadFinished_) {
    load (item);
  }
}

void GoogleWebTranslator::applySettings () {
  QSettings settings;
  settings.beginGroup (settings_names::translationGroup);
  translationLanguage_ = settings.value (settings_names::translationLanguage,
                                         settings_values::translationLanguage).toString ();
}

void GoogleWebTranslator::loadStarted () {
  isLoadFinished_ = false;
  isTranslationFinished_ = false;
}

void GoogleWebTranslator::loadFinished (bool ok) {
  isLoadFinished_ = true;
  if (ok && !isTranslationFinished_) {
    return;
  }

  if (!queue_.isEmpty ()) {
    ProcessingItem item = queue_.front ();
    queue_.pop_front ();
    if (ok) {
      QWebElementCollection result = view_->page ()->mainFrame ()->findAllElements ("#result_box > span");
      item.translated = "";
      foreach (const QWebElement &element, result) {
        item.translated += element.toInnerXml () + " ";
      }
      emit translated (item, !item.translated.isEmpty ());
    }
    else {
      emit translated (item, false);
    }
  }

  if (!queue_.isEmpty ()) {
    load (queue_.front ());
  }
}

void GoogleWebTranslator::replyFinished (QNetworkReply *reply) {
  if (reply->url ().toString ().contains ("/translate_a/single")) {
    isTranslationFinished_ = true;
    if (isLoadFinished_) {
      QTimer::singleShot (2000, this, SLOT (loadFinished ()));
    }
  }
}

void GoogleWebTranslator::load (const ProcessingItem &item) {
  ST_ASSERT (!item.recognized.isEmpty ());
  if (translationLanguage_.isEmpty ()) {
    emit error (tr ("Неверные парметры для перевода."));
    return;
  }
  QString translateLanguage = (item.translateLanguage.isEmpty ())
                              ? translationLanguage_ : item.translateLanguage;
  QUrl url (QString ("https://translate.google.com/#auto/%1/%2").arg (translateLanguage, item.recognized));
  view_->setUrl (url);
}

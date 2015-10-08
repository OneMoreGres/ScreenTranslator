#include <QWebView>
#include <QWebFrame>
#include <QSettings>
#include <QNetworkReply>
#include <QFile>

#include "WebTranslator.h"
#include "ProcessingItem.h"
#include "Settings.h"
#include "StAssert.h"
#include "WebTranslatorProxy.h"

WebTranslator::WebTranslator ()
  : QObject (),
  proxy_ (new WebTranslatorProxy (this)), view_ (new QWebView), isReady_ (true) {

  view_->settings ()->setAttribute (QWebSettings::AutoLoadImages, false);
  view_->settings ()->setAttribute (QWebSettings::DeveloperExtrasEnabled, true);

  connect (view_, SIGNAL (loadFinished (bool)), SLOT (loadFinished (bool)));
  connect (view_->page ()->mainFrame (), SIGNAL (javaScriptWindowObjectCleared ()),
           this, SLOT (addProxyToView ()));
  connect (view_->page ()->networkAccessManager (), SIGNAL (finished (QNetworkReply *)),
           this, SLOT (replyFinished (QNetworkReply *)));

  translationTimeout_.setSingleShot (true);
  connect (&translationTimeout_, SIGNAL (timeout ()), SLOT (abortTranslation ()));

  connect (proxy_, SIGNAL (error (QString)), SLOT (proxyError (QString)));
  connect (proxy_, SIGNAL (translated (QString)), SLOT (proxyTranslated (QString)));

  applySettings ();
}

WebTranslator::~WebTranslator () {
  delete view_;
}

void WebTranslator::addProxyToView () {
  view_->page ()->mainFrame ()->addToJavaScriptWindowObject ("st_wtp", proxy_);
  view_->page ()->mainFrame ()->evaluateJavaScript (script_);
}

void WebTranslator::translate (ProcessingItem item) {
  queue_.push_back (item);
  translateQueued ();
}

void WebTranslator::translateQueued () {
  if (isReady_ && !script_.isEmpty () && !queue_.isEmpty ()) {
    isReady_ = false;
    runScriptForItem (queue_.first ());
    translationTimeout_.start ();
  }
}

void WebTranslator::runScriptForItem (const ProcessingItem &item) {
  ST_ASSERT (!script_.isEmpty ());
  proxy_->setItem (item);
  view_->page ()->mainFrame ()->evaluateJavaScript ("translate();");
}

void WebTranslator::proxyError (const QString &message) {
  emit error (message);
  finishTranslation ();
}

void WebTranslator::proxyTranslated (const QString &text) {
  if (!queue_.isEmpty () && queue_.first ().recognized == proxy_->sourceText ()) {
    ProcessingItem &item = queue_.first ();
    item.translated = text;
    emit translated (item);
  }
  finishTranslation ();
}

void WebTranslator::abortTranslation () {
  emit error (tr ("Перевод отменен по таймауту."));
  finishTranslation ();
}

void WebTranslator::loadFinished (bool ok) {
  if (!ok) {
    QString url = view_->url ().toString ();
    emit error (tr ("Ошибка загрузки страницы (%1) для перевода.").arg (url));
    finishTranslation ();
  }
}

void WebTranslator::finishTranslation () {
  translationTimeout_.stop ();
  if (!queue_.isEmpty ()) {
    queue_.pop_front ();
  }
  isReady_ = true;
  translateQueued ();
}

void WebTranslator::replyFinished (QNetworkReply *reply) {
  emit proxy_->resourceLoaded (reply->url ().toString ());
}

void WebTranslator::applySettings () {
  QSettings settings;
  settings.beginGroup (settings_names::translationGroup);
#define GET(NAME) settings.value (settings_names::NAME, settings_values::NAME)
  translationTimeout_.setInterval (GET (translationTimeout).toInt () * 1000);
#undef GET

  QFile f ("translators/google.js");
  if (f.open (QFile::ReadOnly)) {
    script_ = QString::fromUtf8 (f.readAll ());
    if (script_.isEmpty ()) {
      emit error (tr ("Пустой сценарий для перевода. Перевод недоступен."));
    }
  }
  else {
    emit error (tr ("Не считан сценарий для перевода. Перевод недоступен."));
  }
}

void WebTranslator::setDebugMode (bool isOn) {
  view_->setVisible (isOn);
}

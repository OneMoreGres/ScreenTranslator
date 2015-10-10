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
#include "TranslatorHelper.h"

WebTranslator::WebTranslator ()
  : QObject (),
  proxy_ (new WebTranslatorProxy (this)), view_ (new QWebView),
  translatorHelper_ (new TranslatorHelper), isReady_ (true) {

  view_->settings ()->setAttribute (QWebSettings::AutoLoadImages, false);
  view_->settings ()->setAttribute (QWebSettings::DeveloperExtrasEnabled, true);
  view_->settings ()->setAttribute (QWebSettings::LocalStorageEnabled, true);

  connect (view_, SIGNAL (loadFinished (bool)), SLOT (loadFinished (bool)));
  connect (view_->page ()->mainFrame (), SIGNAL (javaScriptWindowObjectCleared ()),
           this, SLOT (addProxyToView ()));
  connect (view_->page ()->networkAccessManager (), SIGNAL (finished (QNetworkReply *)),
           this, SLOT (replyFinished (QNetworkReply *)));

  translationTimeout_.setSingleShot (true);
  connect (&translationTimeout_, SIGNAL (timeout ()), SLOT (abortTranslation ()));

  connect (proxy_, SIGNAL (translated (QString)), SLOT (proxyTranslated (QString)));

  // Delay because it can emit signal that is not connected yet.
  QTimer::singleShot (500, this, SLOT (applySettings ()));
}

WebTranslator::~WebTranslator () {
  delete translatorHelper_;
  delete view_;
}

void WebTranslator::addProxyToView () {
  view_->page ()->mainFrame ()->addToJavaScriptWindowObject ("st_wtp", proxy_);
  view_->page ()->mainFrame ()->evaluateJavaScript (translatorHelper_->currentScript ());
}

void WebTranslator::translate (ProcessingItem item) {
  if (!item.isValid () || item.translateLanguage.isEmpty ()) {
    emit translated (item);
    return;
  }
  queue_.push_back (item);
  translateQueued ();
}

void WebTranslator::translateQueued () {
  if (isReady_ && !queue_.isEmpty ()) {
    translatorHelper_->newItem ();
    proxy_->setItem (queue_.first ());
    if (!tryNextTranslator (true)) {
      return;
    }
  }
}

void WebTranslator::proxyTranslated (const QString &text) {
  if (!queue_.isEmpty () && queue_.first ().recognized == proxy_->sourceText ()) {
    if (text.isEmpty () && tryNextTranslator ()) {
      return;
    }
    ProcessingItem &item = queue_.first ();
    item.translated = text;
    emit translated (item);
  }
  finishTranslation (false);
}

void WebTranslator::abortTranslation () {
  if (!tryNextTranslator ()) {
    emit error (tr ("Перевод отменен по таймауту."));
    finishTranslation ();
  }
}

void WebTranslator::loadFinished (bool ok) {
  if (!ok && !tryNextTranslator ()) {
    QString url = view_->url ().toString ();
    emit error (tr ("Ошибка загрузки страницы (%1) для перевода.").arg (url));
    finishTranslation ();
  }
}

void WebTranslator::finishTranslation (bool markAsTranslated) {
  translationTimeout_.stop ();
  view_->stop ();
  if (!queue_.isEmpty ()) {
    if (markAsTranslated) {
      emit translated (queue_.first ());
    }
    queue_.pop_front ();
  }
  isReady_ = true;
  translateQueued ();
}

bool WebTranslator::tryNextTranslator (bool firstTime) {
  QString script = firstTime ? translatorHelper_->currentScript ()
                   : translatorHelper_->nextScript ();
  if (script.isEmpty ()) {
    return false;
  }
  translationTimeout_.stop ();
  view_->stop ();
  addProxyToView ();
  view_->page ()->mainFrame ()->evaluateJavaScript ("translate();");
  isReady_ = false;
  translationTimeout_.start ();
  return true;
}

void WebTranslator::replyFinished (QNetworkReply *reply) {
  emit proxy_->resourceLoaded (reply->url ().toString ());
}

void WebTranslator::applySettings () {
  QSettings settings;
  settings.beginGroup (settings_names::translationGroup);
#define GET(NAME) settings.value (settings_names::NAME, settings_values::NAME)
  translationTimeout_.setInterval (GET (translationTimeout).toInt () * 1000);
  translatorHelper_->loadScripts ();
  if (!translatorHelper_->gotScripts ()) {
    emit error (tr ("Нет сценариев для перевода. Измените настройки."));
  }
  bool debugMode = GET (translationDebugMode).toBool ();
  setDebugMode (debugMode);
#undef GET
}

void WebTranslator::setDebugMode (bool isOn) {
  view_->setVisible (isOn);
}

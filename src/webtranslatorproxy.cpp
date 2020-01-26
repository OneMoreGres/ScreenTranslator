#include "webtranslatorproxy.h"
#include "processingitem.h"

WebTranslatorProxy::WebTranslatorProxy (QObject *parent)
  : QObject (parent) {
}

void WebTranslatorProxy::setItem (const ProcessingItem &item) {
  sourceText_ = item.recognized;
  sourceLanguage_ = item.ocrLanguage;
  resultLanguage_ = item.translateLanguage;
}

const QString &WebTranslatorProxy::sourceText () const {
  return sourceText_;
}

const QString &WebTranslatorProxy::sourceLanguage () const {
  return sourceLanguage_;
}

const QString &WebTranslatorProxy::resultLanguage () const {
  return resultLanguage_;
}


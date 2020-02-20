#include "webpageproxy.h"
#include "webpage.h"

WebPageProxy::WebPageProxy(WebPage &page)
  : page_(page)
{
}

void WebPageProxy::setTranslated(const QString &result)
{
  page_.setTranslated(result);
}

void WebPageProxy::setFailed(const QString &error)
{
  page_.setFailed(error);
}

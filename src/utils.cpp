#include <QNetworkProxy>

#include "utils.h"

QString encode (const QString &source) {
  if (source.isEmpty ()) {
    return source;
  }
  char encKeys[] = {14, 26, 99, 43};
  std::string result = source.toStdString ();
  for (int i = 0, end = result.size (); i < end; ++i) {
    result [i] = result[i] ^ encKeys[ i % sizeof(encKeys)];
  }
  return QString::fromUtf8 (result.data ());
}

QList<int> proxyTypeOrder () {
  QList<int> proxyOrder;
  proxyOrder << QNetworkProxy::NoProxy << QNetworkProxy::Socks5Proxy << QNetworkProxy::HttpProxy;
  return proxyOrder;
}

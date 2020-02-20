#pragma once

#include <QObject>

class WebPage;

class WebPageProxy : public QObject
{
  Q_OBJECT
public:
  explicit WebPageProxy(WebPage& page);

signals:
  void terminated();
  void translate(const QString& text, const QString& from, const QString& to);

public slots:
  void setTranslated(const QString& result);
  void setFailed(const QString& error);

private:
  WebPage& page_;
};

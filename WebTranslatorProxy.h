#ifndef WEBTRANSLATORPROXY_H
#define WEBTRANSLATORPROXY_H

#include <QObject>

class ProcessingItem;

/*!
 * \brief Proxy class between WebTranslator and QWebView.
 */
class WebTranslatorProxy : public QObject {
  Q_OBJECT
  Q_PROPERTY (QString sourceText READ sourceText)
  Q_PROPERTY (QString sourceLanguage READ sourceLanguage)
  Q_PROPERTY (QString resultLanguage READ resultLanguage)

  public:
    explicit WebTranslatorProxy (QObject *parent = 0);

    void setItem (const ProcessingItem &item);

    const QString &sourceText () const;
    const QString &sourceLanguage () const;
    const QString &resultLanguage () const;

  signals:
    void translated (const QString &text);
    void error (const QString &message);

    void resourceLoaded (const QString &url);

  private:
    QString sourceText_;
    QString sourceLanguage_;
    QString resultLanguage_;
};

#endif // WEBTRANSLATORPROXY_H

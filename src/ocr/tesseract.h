#pragma once

#include "stfwd.h"

#include <QString>

#include <memory>

class QPixmap;
class Task;

class Tesseract
{
public:
  Tesseract(const LanguageId& language, const QString& tessdataPath,
            const QString& tesseractLibrary);
  ~Tesseract();

  QString recognize(const QPixmap& source);
  bool isValid() const;
  const QString& error() const;

  static QStringList availableLanguageNames(const QString& path);

private:
  class Wrapper;
  void init(const LanguageId& language, const QString& tessdataPath);

  const QString tesseractLibrary_;
  std::unique_ptr<Wrapper> engine_;
  QString error_;
};

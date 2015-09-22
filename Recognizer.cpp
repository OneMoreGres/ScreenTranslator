#include "Recognizer.h"

#include <tesseract/baseapi.h>

#include <QDebug>
#include <QSettings>

#include "Settings.h"
#include "ImageProcessing.h"
#include "StAssert.h"

Recognizer::Recognizer (QObject *parent) :
  QObject (parent),
  engine_ (NULL), imageScale_ (0) {
  applySettings ();
}

void Recognizer::applySettings () {
  QSettings settings;
  settings.beginGroup (settings_names::recogntionGroup);

  tessDataDir_ = settings.value (settings_names::tessDataPlace,
                                 settings_values::tessDataPlace).toString ();
  if (tessDataDir_.right (1) != "/") {
    tessDataDir_ += "/";
  }
  ocrLanguage_ = settings.value (settings_names::ocrLanguage,
                                 settings_values::ocrLanguage).toString ();
  imageScale_ = settings.value (settings_names::imageScale,
                                settings_values::imageScale).toInt ();

  initEngine (engine_, ocrLanguage_);
}

bool Recognizer::initEngine (tesseract::TessBaseAPI * &engine, const QString &language) {
  if (tessDataDir_.isEmpty () || language.isEmpty ()) {
    emit error (tr ("Неверные параметры для OCR"));
    return false;
  }
  if (engine != NULL) {
    delete engine;
  }
  engine = new tesseract::TessBaseAPI ();
  int result = engine->Init (qPrintable (tessDataDir_), qPrintable (language),
                             tesseract::OEM_DEFAULT);
  if (result != 0) {
    emit error (tr ("Ошибка инициализации OCR: %1").arg (result));
    delete engine;
    engine = NULL;
    return false;
  }
  return true;
}

void Recognizer::recognize (ProcessingItem item) {
  ST_ASSERT (!item.source.isNull ());
  bool isCustomLanguage = (!item.ocrLanguage.isEmpty () &&
                           item.ocrLanguage != ocrLanguage_);
  tesseract::TessBaseAPI *engine = (isCustomLanguage) ? NULL : engine_;
  if (engine == NULL) {
    QString language = (isCustomLanguage) ? item.ocrLanguage : ocrLanguage_;
    if (!initEngine (engine, language)) {
      return;
    }
  }

  Pix *image = prepareImage (item.source.toImage (), imageScale_);
  ST_ASSERT (image != NULL);
  engine->SetImage (image);
  char *outText = engine->GetUTF8Text ();
  engine->Clear ();
  cleanupImage (&image);

  QString result = QString (outText).trimmed ();
  delete [] outText;
  if (isCustomLanguage) {
    delete engine;
  }

  if (!result.isEmpty ()) {
    item.recognized = result;
    emit recognized (item);
  }
  else {
    emit error (tr ("Текст не распознан."));
  }
}

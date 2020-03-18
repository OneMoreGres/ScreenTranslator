#include "tesseract.h"
#include "debug.h"
#include "languagecodes.h"
#include "task.h"

#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

#include <QBuffer>
#include <QDir>

#if defined(Q_OS_LINUX)
#include <fstream>
static qint64 getFreeMemory()
{
  std::string token;
  std::ifstream file("/proc/meminfo");
  qint64 freeMem = 0;
  while (file >> token) {
    if (token == "MemFree:" || token == "Buffers:" || token == "Cached:") {
      unsigned long mem = 0;
      freeMem += (file >> mem) ? mem : 0;
    }
  }
  return freeMem * 1024;
}
#elif defined(Q_OS_WIN)
#include <windows.h>
#undef min
#undef max
static qint64 getFreeMemory()
{
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof(statex);
  if (GlobalMemoryStatusEx(&statex)) {
    return statex.ullAvailPhys;
  }
  return -1;
}
#endif

static Pix *convertImage(const QImage &image)
{
  QBuffer buffer;
  buffer.open(QIODevice::WriteOnly);
  image.save(&buffer, "BMP");
  const auto &data = buffer.data();
  return pixReadMemBmp(reinterpret_cast<const l_uint8 *>(data.constData()),
                       data.size());
}

static QImage convertImage(Pix &image)
{
  l_uint8 *buffer = nullptr;
  size_t len = 0;
  pixWriteMemBmp(&buffer, &len, &image);

  QImage result;
  result.loadFromData(static_cast<uchar *>(buffer), len);
  return result;
}

static double getScale(Pix *source)
{
  SOFT_ASSERT(source, return -1.0);

  const auto xRes = pixGetXRes(source);
  const auto yRes = pixGetYRes(source);
  if (xRes * yRes == 0)
    return -1.0;

  const auto preferredScale = std::max(500.0 / std::min(xRes, yRes), 1.0);
  if (preferredScale <= 1.0)
    return -1.0;

  const auto maxScaleX = std::numeric_limits<int>::max() / double(source->w);
  const auto scaleX = std::min(preferredScale, maxScaleX);
  const auto maxScaleY = std::numeric_limits<int>::max() / double(source->h);
  const auto scaleY = std::min(preferredScale, maxScaleY);
  auto scale = std::min(scaleX, scaleY);

  const auto availableMemory = getFreeMemory() * 0.95;
  if (availableMemory < 1)
    return -1.0;

  const auto actualSize = source->w * source->h * source->d / 8;
  const auto maxScaleMemory = availableMemory / actualSize;
  scale = std::min(scale, maxScaleMemory);

  return scale;
}

static Pix *prepareImage(const QImage &image)
{
  auto pix = convertImage(image);
  SOFT_ASSERT(pix, return nullptr);

  auto gray = pixConvertRGBToGray(pix, 0.0, 0.0, 0.0);
  SOFT_ASSERT(gray, return nullptr);
  pixDestroy(&pix);

  auto scaleSource = gray;
  auto scaled = scaleSource;

  if (const auto scale = getScale(scaleSource); scale > 1.0) {
    scaled = pixScale(scaleSource, scale, scale);
    if (!scaled)
      scaled = scaleSource;
  }

  if (scaled != scaleSource)
    pixDestroy(&scaleSource);

  return scaled;
}

static void cleanupImage(Pix **image)
{
  pixDestroy(image);
}

Tesseract::Tesseract(const LanguageId &language, const QString &tessdataPath)
{
  SOFT_ASSERT(!tessdataPath.isEmpty(), return );
  SOFT_ASSERT(!language.isEmpty(), return );

  init(language, tessdataPath);
}

Tesseract::~Tesseract() = default;

void Tesseract::init(const LanguageId &language, const QString &tessdataPath)
{
  SOFT_ASSERT(!engine_, return );

  LanguageCodes languages;
  auto langCodes = languages.findById(language);
  if (!langCodes) {
    error_ = QObject::tr("unknown recognition language: %1").arg(language);
    return;
  }

  engine_ = std::make_unique<tesseract::TessBaseAPI>();

  auto result =
      engine_->Init(qPrintable(tessdataPath), qPrintable(langCodes->tesseract),
                    tesseract::OEM_DEFAULT);
  if (result == 0)
    return;

  error_ = QObject::tr("troubles with tessdata");
  engine_.reset();
  return;
}

const QString &Tesseract::error() const
{
  return error_;
}

QStringList Tesseract::availableLanguageNames(const QString &path)
{
  if (path.isEmpty())
    return {};

  QDir dir(path);
  if (!dir.exists())
    return {};

  LanguageIds names;
  LanguageCodes languages;

  const auto files = dir.entryList({"*.traineddata"}, QDir::Files);
  for (const auto &file : files) {
    const auto lang = file.left(file.indexOf("."));
    if (const auto bundle = languages.findByTesseract(lang))
      names.append(QObject::tr(bundle->name));
    else
      names.append(lang);
  }

  if (names.isEmpty())
    return {};

  return names;
}

QString Tesseract::recognize(const QPixmap &source)
{
  SOFT_ASSERT(engine_, return {});
  SOFT_ASSERT(!source.isNull(), return {});

  error_.clear();

  Pix *image = prepareImage(source.toImage());
  SOFT_ASSERT(image != NULL, return {});
  engine_->SetImage(image);
  char *outText = engine_->GetUTF8Text();
  engine_->Clear();
  cleanupImage(&image);

  QString result = QString(outText).trimmed();
  delete[] outText;

  if (result.isEmpty())
    error_ = QObject::tr("Failed to recognize text");
  return result;
}

bool Tesseract::isValid() const
{
  return engine_.get();
}

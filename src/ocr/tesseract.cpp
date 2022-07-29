#include "tesseract.h"
#include "debug.h"
#include "languagecodes.h"
#include "task.h"

#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

#include <QBuffer>
#include <QDir>
#include <QLibrary>

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
  result.loadFromData(static_cast<uchar *>(buffer), int(len));
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

// Smart pointer for Pix
class PixGuard
{
public:
  explicit PixGuard(Pix *pix = nullptr)
    : pix_(pix)
  {
  }
  ~PixGuard()
  {
    if (pix_)
      pixDestroy(&pix_);
  }
  void operator=(Pix *pix)
  {
    if (!pix)
      return;
    if (pix_)
      pixDestroy(&pix_);
    pix_ = pix;
  }
  operator Pix *() { return pix_; }
  Pix *operator->() { return pix_; }
  Pix *&get() { return pix_; }
  Pix *take()
  {
    auto ret = pix_;
    pix_ = nullptr;
    return ret;
  }
  void trace(const QString &name) const
  {
    LTRACE() << qPrintable(name) << pix_;
#if 0
    if (!pix_)
      return;
    auto fileName = name + ".png";
    fileName.replace(' ', "_");
    convertImage(*pix_).save(fileName);
#endif
  }

private:
  Pix *pix_;

  Q_DISABLE_COPY(PixGuard);
};

static Pix *prepareImage(const QImage &image)
{
  auto pix = PixGuard(convertImage(image));
  SOFT_ASSERT(pix, return nullptr);
  pix.trace("Pix 1 Converted");

  {
    pix = pixConvertRGBToGray(pix, 0.0, 0.0, 0.0);
    pix.trace("Pix 2 Gray");
  }

  if (const auto scale = getScale(pix); scale > 1.0) {
    pix = pixScaleGrayLI(pix, scale, scale);
    pix.trace("Pix 3 Scaled");
  }

  l_int32 otsuSx = 5000;
  l_int32 otsuSy = 5000;
  l_int32 otsuSmoothx = 0;
  l_int32 otsuSmoothy = 0;
  l_float32 otsuScorefract = 0.1f;

  {
    PixGuard otsu;
    pixOtsuAdaptiveThreshold(pix, otsuSx, otsuSy, otsuSmoothx, otsuSmoothy,
                             otsuScorefract, nullptr, &otsu.get());
    pix.trace("Pix 4 Test Color Otsu");

    // Get the average intensity of the border pixels,
    // with average of 0.0 being completely white and 1.0 being completely black
    // Top
    auto avg = pixAverageOnLine(otsu, 0, 0, otsu->w - 1, 0, 1);
    // Bottom
    avg += pixAverageOnLine(otsu, 0, otsu->h - 1, otsu->w - 1, otsu->h - 1, 1);
    // Left
    avg += pixAverageOnLine(otsu, 0, 0, 0, otsu->h - 1, 1);
    // Right
    avg += pixAverageOnLine(otsu, otsu->w - 1, 0, otsu->w - 1, otsu->h - 1, 1);
    avg /= 4.0f;

    // If background is dark
    l_float32 threshold = 0.5f;
    if (avg > threshold) {
      pix = pixInvert(nullptr, pix);
      pix.trace("Pix 5 Inverted");
    }
  }

  {
    l_int32 usm_halfwidth = 5;
    l_float32 usm_fract = 2.5f;
    pix = pixUnsharpMaskingGray(pix, usm_halfwidth, usm_fract);
    pix.trace("Pix 6 Unshapred");
  }

  {
    pixOtsuAdaptiveThreshold(pix, otsuSx, otsuSy, otsuSmoothx, otsuSmoothy, 0.0,
                             nullptr, &pix.get());
    pix.trace("Pix 7 Binarized");
  }

  pix.trace("Pix 8 Result");

  return pix.take();
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
  SOFT_ASSERT(!api_, return );

  api_ = std::make_unique<tesseract::TessBaseAPI>();
  LTRACE() << "Created Tesseract api" << api_.get();

  const auto tesseractName = LanguageCodes::tesseract(language);
  auto result = api_->Init(qPrintable(tessdataPath), qPrintable(tesseractName),
                           tesseract::OcrEngineMode::OEM_DEFAULT);
  LTRACE() << "Inited Tesseract api" << result;
  if (result == 0)
    return;

  api_->SetPageSegMode(tesseract::PageSegMode::PSM_AUTO);

  error_ = QObject::tr("init failed");
  api_.reset();
  LTRACE() << "Cleared Tesseract api";
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

  const auto files = dir.entryList({"*.traineddata"}, QDir::Files);
  for (const auto &file : files) {
    const auto lang = file.left(file.indexOf("."));
    const auto name = LanguageCodes::name(LanguageCodes::idForTesseract(lang));
    names.append(name);
  }

  if (names.isEmpty())
    return {};

  return names;
}

QString Tesseract::recognize(const QPixmap &source)
{
  SOFT_ASSERT(api_, return {});
  SOFT_ASSERT(!source.isNull(), return {});

  error_.clear();

  PixGuard image(prepareImage(source.toImage()));
  SOFT_ASSERT(image, return {});
  LTRACE() << "Preprocessed Pix for OCR" << image;

  api_->SetImage(image);
  LTRACE() << "Set Pix to engine";

  const auto outText = api_->GetUTF8Text();
  LTRACE() << "Received recognized text";

  api_->Clear();
  LTRACE() << "Cleared engine";

  const auto result = QString(outText).trimmed();

  delete[] outText;
  LTRACE() << "Cleared recognized text buffer";

  if (result.isEmpty())
    error_ = QObject::tr("Failed to recognize text or no text selected");
  return result;
}

bool Tesseract::isValid() const
{
  return api_.get();
}

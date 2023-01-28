#include "hunspellcorrector.h"
#include "debug.h"
#include "languagecodes.h"
#include "settings.h"

#include <hunspell/hunspell.hxx>

#include <QDir>
#include <QRegularExpression>
#include <QStringConverter>

static int levenshteinDistance(const QString &source, const QString &target)
{
  if (source == target)
    return 0;

  const auto sourceCount = source.size();
  const auto targetCount = target.size();

  if (sourceCount == 0)
    return targetCount;

  if (targetCount == 0)
    return sourceCount;

  if (sourceCount > targetCount)
    return levenshteinDistance(target, source);

  QVector<int> previousColumn;
  previousColumn.reserve(targetCount + 1);
  for (auto i = 0; i < targetCount + 1; ++i) previousColumn.append(i);

  QVector<int> column(targetCount + 1, 0);
  for (auto i = 0; i < sourceCount; ++i) {
    column[0] = i + 1;
    for (auto j = 0; j < targetCount; ++j) {
      column[j + 1] = std::min(
          {1 + column.at(j), 1 + previousColumn.at(1 + j),
           previousColumn.at(j) + ((source.at(i) == target.at(j)) ? 0 : 1)});
    }
    column.swap(previousColumn);
  }

  return previousColumn.at(targetCount);
}

HunspellCorrector::HunspellCorrector(const LanguageId &language,
                                     const QString &dictPath)
{
  const auto name = LanguageCodes::iso639_1(language);
  init(dictPath + QLatin1Char('/') + name);
}

HunspellCorrector::~HunspellCorrector() = default;

const QString &HunspellCorrector::error() const
{
  return error_;
}

bool HunspellCorrector::isValid() const
{
  return engine_.get();
}

void HunspellCorrector::init(const QString &path)
{
  SOFT_ASSERT(!engine_, return );

  QDir dir(path);
  if (!dir.exists()) {
    error_ = QObject::tr("Hunspell path not exists\n%1").arg(path);
    return;
  }

  QString aff;
  QStringList dics;
  for (const auto &file : dir.entryList(QDir::Filter::Files)) {
    if (file.endsWith(".aff")) {
      aff = dir.absoluteFilePath(file);
      continue;
    }
    if (file.endsWith(".dic")) {
      dics.append(dir.absoluteFilePath(file));
      continue;
    }
  }

  if (aff.isEmpty() || dics.isEmpty()) {
    error_ = QObject::tr("No .aff or .dic files for hunspell\nin %1").arg(path);
    return;
  }

  engine_ =
      std::make_unique<Hunspell>(qPrintable(aff), qPrintable(dics.first()));
  LTRACE() << "Created hunspell instance";

  dics.pop_front();
  if (!dics.isEmpty()) {
    for (const auto &dic : dics) engine_->add_dic(qPrintable(dic));
    LTRACE() << "Loaded hunspell dicts" << dics;
  }
}

QString HunspellCorrector::correct(const QString &original)
{
  SOFT_ASSERT(engine_, return original);

  const auto encoding =
      QStringConverter::encodingForName(engine_->get_dict_encoding().c_str());
  SOFT_ASSERT(encoding, return original);
  auto codec = QStringEncoder(*encoding);

  QString result;

  QString word;
  QString separator;

  for (auto i = 0ll, end = original.size(); i < end; ++i) {
    const auto ch = original[i];
    if (ch.isPunct() || ch.isSpace()) {
      if (!word.isEmpty()) {
        correctWord(word, codec);
        result += word;
        word.clear();
      }
      separator += ch;
      continue;
    }

    if (!ch.isLetter() && word.isEmpty()) {
      separator += ch;
      continue;
    }

    if (!separator.isEmpty()) {
      result += separator;
      separator.clear();
    }
    word += ch;
  }

  if (!word.isEmpty()) {
    correctWord(word, codec);
    result += word;
  }
  result += separator;

  return result;
}

void HunspellCorrector::correctWord(QString &word, QStringEncoder &codec) const
{
  if (word.isEmpty())
    return;

  const auto stdWord = codec(word).data.toStdString();
  if (engine_->spell(stdWord))
    return;

  const auto suggestions = engine_->suggest(stdWord);
  if (suggestions.empty())
    return;

  const auto suggestion = QByteArray::fromStdString(suggestions.front());
  const auto distance = levenshteinDistance(word, suggestion);
  const auto maxDistance = std::max(int(word.size() * 0.2), 1);
  LTRACE() << "hunspell" << word << suggestion << "distances" << distance
           << maxDistance;

  if (distance <= maxDistance)
    word = suggestion;
}

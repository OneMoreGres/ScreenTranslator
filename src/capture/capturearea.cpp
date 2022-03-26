#include "capturearea.h"
#include "settings.h"
#include "task.h"

CaptureArea::CaptureArea(const QRect &rect, const Settings &settings)
  : rect_(rect)
  , doTranslation_(settings.doTranslation)
  , useHunspell_(settings.useHunspell)
  , sourceLanguage_(settings.sourceLanguage)
  , targetLanguage_(settings.targetLanguage)
  , translators_(settings.translators)
{
}

TaskPtr CaptureArea::task(const QPixmap &pixmap,
                          const QPoint &pixmapOffset) const
{
  if (pixmap.isNull() || !isValid())
    return {};

  auto task = std::make_shared<Task>();
  task->generation = generation_;
  task->useHunspell = useHunspell_;
  task->captured = pixmap.copy(rect_);
  task->capturePoint = pixmapOffset + rect_.topLeft();
  task->sourceLanguage = sourceLanguage_;
  if (task->sourceLanguage.isEmpty())
    task->error += QObject::tr("No source language set");

  if (doTranslation_ && !translators_.isEmpty()) {
    task->targetLanguage = targetLanguage_;
    task->translators = translators_;
    if (task->targetLanguage.isEmpty()) {
      task->error += (task->error.isEmpty() ? "" : ", ") +
                     QObject::tr("No target language set");
    }
  }

  return task;
}

void CaptureArea::setGeneration(uint generation)
{
  generation_ = generation;
}

bool CaptureArea::isValid() const
{
  return !(rect_.width() < 3 || rect_.height() < 3);
}

const QRect &CaptureArea::rect() const
{
  return rect_;
}

void CaptureArea::setRect(const QRect &rect)
{
  rect_ = rect;
}

QString CaptureArea::toolTip() const
{
  return doTranslation_ ? sourceLanguage_ + "->" + targetLanguage_
                        : sourceLanguage_;
}

bool CaptureArea::isLocked() const
{
  return isLocked_;
}

#include "geometryutils.h"

#include <QRect>

namespace service
{
namespace geometry
{
QPoint cornerAtPoint(const QPoint &corner, const QSize &size,
                     const QRect &boundRect)
{
  auto clamped = corner;
  if (!boundRect.contains(clamped)) {
    const auto x = std::clamp(clamped.x(), boundRect.left(), boundRect.right());
    const auto y = std::clamp(clamped.y(), boundRect.top(), boundRect.bottom());
    clamped = QPoint(x, y);
  }

  const auto zeroCorner = clamped - boundRect.topLeft();
  const auto boundWidth = boundRect.width();
  const auto boundHeight = boundRect.height();

  QPoint result;
  if (boundWidth <= size.width()) {  // not fits
    result.rx() = 0;
  } else if (boundWidth - zeroCorner.x() >= size.width()) {  // enough on right
    result.rx() = zeroCorner.x();
  } else if (zeroCorner.x() >= size.width()) {  // enough on left
    result.rx() = zeroCorner.x() - size.width() + 1;
  } else {  // not enough on both sides
    result.rx() =
        zeroCorner.x() >= boundWidth / 2 ? 0 : boundWidth - size.width() + 1;
  }

  if (boundHeight <= size.height()) {
    result.ry() = 0;
  } else if (boundHeight - zeroCorner.y() >= size.height()) {
    result.ry() = zeroCorner.y();
  } else if (zeroCorner.y() >= size.height()) {
    result.ry() = zeroCorner.y() - size.height() + 1;
  } else {
    result.ry() =
        zeroCorner.y() >= boundHeight / 2 ? 0 : boundHeight - size.height() + 1;
  }

  return result + boundRect.topLeft();
}

}  // namespace geometry
}  // namespace service

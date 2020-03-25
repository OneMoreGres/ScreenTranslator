#pragma once

class QRect;
class QPoint;
class QSize;

namespace service
{
namespace geometry
{
QPoint cornerAtPoint(const QPoint& corner, const QSize& size,
                     const QRect& boundRect);
}
}  // namespace service

#include "ProcessingItem.h"

bool ProcessingItem::isValid () const {
  bool valid = true;
  valid &= (!screenPos.isNull ());
  valid &= (!source.isNull ());
  valid &= (!recognized.isEmpty ());
  return valid;
}

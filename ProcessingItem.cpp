#include "ProcessingItem.h"

bool ProcessingItem::isValid (bool checkOnlyInput) const {
  bool valid = true;
  valid &= (!screenPos.isNull ());
  valid &= (!source.isNull ());
  valid &= (!ocrLanguage.isEmpty ());
  if (!checkOnlyInput) {
    valid &= (!recognized.isEmpty ());
  }
  return valid;
}

#ifndef PROCESSINGITEM_H
#define PROCESSINGITEM_H

#include <QPixmap>

struct ProcessingItem {
  ProcessingItem ();
  QPoint screenPos;
  QPixmap source;
  QString recognized;
  QString translated;

  QString ocrLanguage;
  QString sourceLanguage;
  QString translateLanguage;

  Qt::KeyboardModifiers modifiers;
  bool swapLanguages_;

  bool isValid (bool checkOnlyInput = false) const;
};
Q_DECLARE_METATYPE (ProcessingItem)

#endif // PROCESSINGITEM_H

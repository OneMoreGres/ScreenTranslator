#ifndef PROCESSINGITEM_H
#define PROCESSINGITEM_H

#include <QPixmap>

struct ProcessingItem {
  QPoint screenPos;
  QPixmap source;
  QString recognized;
  QString translated;

  QString ocrLanguage;
  QString sourceLanguage;
  QString translateLanguage;

  Qt::KeyboardModifiers modifiers;

  bool isValid () const;
};
Q_DECLARE_METATYPE (ProcessingItem)

#endif // PROCESSINGITEM_H

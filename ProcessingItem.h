#ifndef PROCESSINGITEM_H
#define PROCESSINGITEM_H

#include <QPixmap>

struct ProcessingItem
{
    QPoint screenPos;
    QPixmap source;
    QString recognized;
    QString translated;
};
Q_DECLARE_METATYPE(ProcessingItem)

#endif // PROCESSINGITEM_H

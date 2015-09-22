#ifndef GLOBALACTIONHELPER_H
#define GLOBALACTIONHELPER_H

// Some functions copied from QXT lib

#include <QAbstractNativeEventFilter>
#include <QAction>

class GlobalActionHelper : public QAbstractNativeEventFilter {
  public:
    bool nativeEventFilter (const QByteArray &eventType, void *message,
                            long *result);

    static void init ();
    static bool makeGlobal (QAction *action);
    static bool removeGlobal (QAction *action);

  private:
    static QHash<QPair<quint32, quint32>, QAction *> actions_;

    static quint32 nativeKeycode (Qt::Key key);
    static quint32 nativeModifiers (Qt::KeyboardModifiers modifiers);
    static bool registerHotKey (quint32 nativeKey, quint32 nativeMods);
    static bool unregisterHotKey (quint32 nativeKey, quint32 nativeMods);

};

#endif // GLOBALACTIONHELPER_H

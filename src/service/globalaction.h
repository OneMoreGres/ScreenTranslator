#pragma once

// Some functions copied from QXT lib

#include <QAbstractNativeEventFilter>
#include <QAction>

namespace service
{
class GlobalAction : public QAbstractNativeEventFilter
{
public:
  bool nativeEventFilter(const QByteArray &eventType, void *message,
                         long *result);

  static void init();
  static bool makeGlobal(QAction *action);
  static bool removeGlobal(QAction *action);
  static bool update(QAction *action, const QKeySequence &newShortcut);

private:
  static QHash<QPair<quint32, quint32>, QAction *> actions_;

  static quint32 nativeKeycode(Qt::Key key);
  static quint32 nativeModifiers(Qt::KeyboardModifiers modifiers);
  static bool registerHotKey(quint32 nativeKey, quint32 nativeMods);
  static bool unregisterHotKey(quint32 nativeKey, quint32 nativeMods);
  static void triggerHotKey(quint32 nativeKey, quint32 nativeMods);

  friend struct ActionAdapter;
};

}  // namespace service

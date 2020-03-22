#include "globalaction.h"
#include "debug.h"

#include <QApplication>

namespace service
{
QHash<QPair<quint32, quint32>, QAction *> GlobalAction::actions_;

void GlobalAction::init()
{
  qApp->installNativeEventFilter(new GlobalAction);
}

bool GlobalAction::makeGlobal(QAction *action)
{
  QKeySequence hotKey = action->shortcut();
  if (hotKey.isEmpty())
    return true;
  Qt::KeyboardModifiers allMods = Qt::ShiftModifier | Qt::ControlModifier |
                                  Qt::AltModifier | Qt::MetaModifier;
  Qt::Key key = hotKey.isEmpty() ? Qt::Key(0)
                                 : Qt::Key((hotKey[0] ^ allMods) & hotKey[0]);
  Qt::KeyboardModifiers mods = hotKey.isEmpty()
                                   ? Qt::KeyboardModifiers(0)
                                   : Qt::KeyboardModifiers(hotKey[0] & allMods);
  const quint32 nativeKey = nativeKeycode(key);
  const quint32 nativeMods = nativeModifiers(mods);
  const bool res = registerHotKey(nativeKey, nativeMods);
  if (res)
    actions_.insert(qMakePair(nativeKey, nativeMods), action);
  else
    LERROR() << "Failed to register global hotkey:" << LARG(hotKey.toString());
  return res;
}

bool GlobalAction::removeGlobal(QAction *action)
{
  QKeySequence hotKey = action->shortcut();
  if (hotKey.isEmpty())
    return true;
  Qt::KeyboardModifiers allMods = Qt::ShiftModifier | Qt::ControlModifier |
                                  Qt::AltModifier | Qt::MetaModifier;
  Qt::Key key = hotKey.isEmpty() ? Qt::Key(0)
                                 : Qt::Key((hotKey[0] ^ allMods) & hotKey[0]);
  Qt::KeyboardModifiers mods = hotKey.isEmpty()
                                   ? Qt::KeyboardModifiers(0)
                                   : Qt::KeyboardModifiers(hotKey[0] & allMods);
  const quint32 nativeKey = nativeKeycode(key);
  const quint32 nativeMods = nativeModifiers(mods);
  if (!actions_.contains(qMakePair(nativeKey, nativeMods)))
    return true;
  const bool res = unregisterHotKey(nativeKey, nativeMods);
  if (res)
    actions_.remove(qMakePair(nativeKey, nativeMods));
  else
    LERROR() << "Failed to unregister global hotkey:" << (hotKey.toString());
  return res;
}

bool GlobalAction::update(QAction *action, const QKeySequence &newShortcut)
{
  if (!action->shortcut().isEmpty())
    removeGlobal(action);
  action->setShortcut(newShortcut);
  return newShortcut.isEmpty() ? true : makeGlobal(action);
}

void GlobalAction::triggerHotKey(quint32 nativeKey, quint32 nativeMods)
{
  QAction *action = actions_.value(qMakePair(nativeKey, nativeMods));
  if (action && action->isEnabled())
    action->activate(QAction::Trigger);
}
}  // namespace service

#ifdef Q_OS_LINUX
#include <X11/Xlib.h>
#include <xcb/xcb_event.h>
#include <QX11Info>

namespace service
{
static bool error = false;

static int customHandler(Display *display, XErrorEvent *event)
{
  Q_UNUSED(display);
  switch (event->error_code) {
    case BadAccess:
    case BadValue:
    case BadWindow:
      if (event->request_code == 33 /* X_GrabKey */ ||
          event->request_code == 34 /* X_UngrabKey */) {
        error = true;
      }
    default: return 0;
  }
}

bool GlobalAction::registerHotKey(quint32 nativeKey, quint32 nativeMods)
{
  Display *display = QX11Info::display();
  Window window = QX11Info::appRootWindow();
  Bool owner = True;
  int pointer = GrabModeAsync;
  int keyboard = GrabModeAsync;
  error = false;
  int (*handler)(Display * display, XErrorEvent * event) =
      XSetErrorHandler(customHandler);
  XGrabKey(display, nativeKey, nativeMods, window, owner, pointer, keyboard);
  // allow numlock
  XGrabKey(display, nativeKey, nativeMods | Mod2Mask, window, owner, pointer,
           keyboard);
  XSync(display, False);
  XSetErrorHandler(handler);
  return !error;
}

bool GlobalAction::unregisterHotKey(quint32 nativeKey, quint32 nativeMods)
{
  Display *display = QX11Info::display();
  Window window = QX11Info::appRootWindow();
  error = false;
  int (*handler)(Display * display, XErrorEvent * event) =
      XSetErrorHandler(customHandler);
  XUngrabKey(display, nativeKey, nativeMods, window);
  // allow numlock
  XUngrabKey(display, nativeKey, nativeMods | Mod2Mask, window);
  XSync(display, False);
  XSetErrorHandler(handler);
  return !error;
}

bool GlobalAction::nativeEventFilter(const QByteArray &eventType, void *message,
                                     long *result)
{
  Q_UNUSED(eventType);
  Q_UNUSED(result);
  xcb_generic_event_t *event = static_cast<xcb_generic_event_t *>(message);
  if (event->response_type == XCB_KEY_PRESS) {
    xcb_key_press_event_t *keyEvent =
        static_cast<xcb_key_press_event_t *>(message);
    const quint32 keycode = keyEvent->detail;
    const quint32 modifiers = keyEvent->state & ~XCB_MOD_MASK_2;
    triggerHotKey(keycode, modifiers);
  }
  return false;
}

quint32 GlobalAction::nativeKeycode(Qt::Key key)
{
  Display *display = QX11Info::display();
  KeySym keySym = XStringToKeysym(qPrintable(QKeySequence(key).toString()));
  return XKeysymToKeycode(display, keySym);
}

quint32 GlobalAction::nativeModifiers(Qt::KeyboardModifiers modifiers)
{
  quint32 native = 0;
  if (modifiers & Qt::ShiftModifier)
    native |= ShiftMask;
  if (modifiers & Qt::ControlModifier)
    native |= ControlMask;
  if (modifiers & Qt::AltModifier)
    native |= Mod1Mask;
  if (modifiers & Qt::MetaModifier)
    native |= Mod4Mask;
  return native;
}

#endif  // ifdef Q_OS_LINUX

#ifdef Q_OS_WIN
#include <qt_windows.h>

namespace service
{
bool GlobalAction::registerHotKey(quint32 nativeKey, quint32 nativeMods)
{
  return RegisterHotKey(0, nativeMods ^ nativeKey, nativeMods, nativeKey);
}

bool GlobalAction::unregisterHotKey(quint32 nativeKey, quint32 nativeMods)
{
  return UnregisterHotKey(0, nativeMods ^ nativeKey);
}

bool GlobalAction::nativeEventFilter(const QByteArray &eventType, void *message,
                                     long *result)
{
  Q_UNUSED(eventType);
  Q_UNUSED(result);
  MSG *msg = static_cast<MSG *>(message);
  if (msg->message == WM_HOTKEY) {
    const quint32 keycode = HIWORD(msg->lParam);
    const quint32 modifiers = LOWORD(msg->lParam);
    triggerHotKey(keycode, modifiers);
  }
  return false;
}

quint32 GlobalAction::nativeKeycode(Qt::Key key)
{
  switch (key) {
    case Qt::Key_Escape: return VK_ESCAPE;
    case Qt::Key_Tab:
    case Qt::Key_Backtab: return VK_TAB;
    case Qt::Key_Backspace: return VK_BACK;
    case Qt::Key_Return:
    case Qt::Key_Enter: return VK_RETURN;
    case Qt::Key_Insert: return VK_INSERT;
    case Qt::Key_Delete: return VK_DELETE;
    case Qt::Key_Pause: return VK_PAUSE;
    case Qt::Key_Print: return VK_PRINT;
    case Qt::Key_Clear: return VK_CLEAR;
    case Qt::Key_Home: return VK_HOME;
    case Qt::Key_End: return VK_END;
    case Qt::Key_Left: return VK_LEFT;
    case Qt::Key_Up: return VK_UP;
    case Qt::Key_Right: return VK_RIGHT;
    case Qt::Key_Down: return VK_DOWN;
    case Qt::Key_PageUp: return VK_PRIOR;
    case Qt::Key_PageDown: return VK_NEXT;
    case Qt::Key_F1: return VK_F1;
    case Qt::Key_F2: return VK_F2;
    case Qt::Key_F3: return VK_F3;
    case Qt::Key_F4: return VK_F4;
    case Qt::Key_F5: return VK_F5;
    case Qt::Key_F6: return VK_F6;
    case Qt::Key_F7: return VK_F7;
    case Qt::Key_F8: return VK_F8;
    case Qt::Key_F9: return VK_F9;
    case Qt::Key_F10: return VK_F10;
    case Qt::Key_F11: return VK_F11;
    case Qt::Key_F12: return VK_F12;
    case Qt::Key_F13: return VK_F13;
    case Qt::Key_F14: return VK_F14;
    case Qt::Key_F15: return VK_F15;
    case Qt::Key_F16: return VK_F16;
    case Qt::Key_F17: return VK_F17;
    case Qt::Key_F18: return VK_F18;
    case Qt::Key_F19: return VK_F19;
    case Qt::Key_F20: return VK_F20;
    case Qt::Key_F21: return VK_F21;
    case Qt::Key_F22: return VK_F22;
    case Qt::Key_F23: return VK_F23;
    case Qt::Key_F24: return VK_F24;
    case Qt::Key_Space: return VK_SPACE;
    case Qt::Key_Asterisk: return VK_MULTIPLY;
    case Qt::Key_Plus: return VK_ADD;
    case Qt::Key_Comma: return VK_SEPARATOR;
    case Qt::Key_Minus: return VK_SUBTRACT;
    case Qt::Key_Slash: return VK_DIVIDE;
    case Qt::Key_MediaNext: return VK_MEDIA_NEXT_TRACK;
    case Qt::Key_MediaPrevious: return VK_MEDIA_PREV_TRACK;
    case Qt::Key_MediaPlay: return VK_MEDIA_PLAY_PAUSE;
    case Qt::Key_MediaStop: return VK_MEDIA_STOP;
    case Qt::Key_VolumeDown: return VK_VOLUME_DOWN;
    case Qt::Key_VolumeUp: return VK_VOLUME_UP;
    case Qt::Key_VolumeMute: return VK_VOLUME_MUTE;

    // numbers
    case Qt::Key_0:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9: return key;

    // letters
    case Qt::Key_A:
    case Qt::Key_B:
    case Qt::Key_C:
    case Qt::Key_D:
    case Qt::Key_E:
    case Qt::Key_F:
    case Qt::Key_G:
    case Qt::Key_H:
    case Qt::Key_I:
    case Qt::Key_J:
    case Qt::Key_K:
    case Qt::Key_L:
    case Qt::Key_M:
    case Qt::Key_N:
    case Qt::Key_O:
    case Qt::Key_P:
    case Qt::Key_Q:
    case Qt::Key_R:
    case Qt::Key_S:
    case Qt::Key_T:
    case Qt::Key_U:
    case Qt::Key_V:
    case Qt::Key_W:
    case Qt::Key_X:
    case Qt::Key_Y:
    case Qt::Key_Z: return key;

    default: return 0;
  }
}

quint32 GlobalAction::nativeModifiers(Qt::KeyboardModifiers modifiers)
{
  // MOD_ALT, MOD_CONTROL, (MOD_KEYUP), MOD_SHIFT, MOD_WIN
  quint32 native = 0;
  if (modifiers & Qt::ShiftModifier)
    native |= MOD_SHIFT;
  if (modifiers & Qt::ControlModifier)
    native |= MOD_CONTROL;
  if (modifiers & Qt::AltModifier)
    native |= MOD_ALT;
  if (modifiers & Qt::MetaModifier)
    native |= MOD_WIN;
  // if (modifiers & Qt::KeypadModifier)
  // if (modifiers & Qt::GroupSwitchModifier)
  return native;
}

#endif  // ifdef Q_OS_WIN

#ifdef Q_OS_MAC
#include <Carbon/Carbon.h>

namespace service
{
static bool isInited = false;
static QHash<QPair<quint32, quint32>, EventHotKeyRef> hotkeyRefs;

struct ActionAdapter {
  static OSStatus macHandler(EventHandlerCallRef /*nextHandler*/,
                             EventRef event, void * /*userData*/)
  {
    EventHotKeyID id;
    GetEventParameter(event, kEventParamDirectObject, typeEventHotKeyID, NULL,
                      sizeof(id), NULL, &id);
    GlobalAction::triggerHotKey(quint32(id.signature), quint32(id.id));
    return noErr;
  }
};

bool GlobalAction::registerHotKey(quint32 nativeKey, quint32 nativeMods)
{
  if (!isInited) {
    EventTypeSpec spec;
    spec.eventClass = kEventClassKeyboard;
    spec.eventKind = kEventHotKeyPressed;
    InstallApplicationEventHandler(&ActionAdapter::macHandler, 1, &spec, NULL,
                                   NULL);
    isInited = true;
  }

  EventHotKeyID id;
  id.signature = nativeKey;
  id.id = nativeMods;

  EventHotKeyRef ref = NULL;
  OSStatus status = RegisterEventHotKey(nativeKey, nativeMods, id,
                                        GetApplicationEventTarget(), 0, &ref);
  if (status != noErr) {
    LERROR() << "RegisterEventHotKey error:" << LARG(status);
    return false;
  } else {
    hotkeyRefs.insert(qMakePair(nativeKey, nativeMods), ref);
    return true;
  }
}

bool GlobalAction::unregisterHotKey(quint32 nativeKey, quint32 nativeMods)
{
  EventHotKeyRef ref = hotkeyRefs.value(qMakePair(nativeKey, nativeMods));
  ASSERT(ref);
  OSStatus status = UnregisterEventHotKey(ref);
  if (status != noErr) {
    LERROR() << "UnregisterEventHotKey error:" << LARG(status);
    return false;
  } else {
    hotkeyRefs.remove(qMakePair(nativeKey, nativeMods));
    return true;
  }
}

bool GlobalAction::nativeEventFilter(const QByteArray & /*eventType*/,
                                     void * /*message*/, long * /*result*/)
{
  return false;
}

quint32 GlobalAction::nativeKeycode(Qt::Key key)
{
  switch (key) {
    case Qt::Key_A: return kVK_ANSI_A;
    case Qt::Key_B: return kVK_ANSI_B;
    case Qt::Key_C: return kVK_ANSI_C;
    case Qt::Key_D: return kVK_ANSI_D;
    case Qt::Key_E: return kVK_ANSI_E;
    case Qt::Key_F: return kVK_ANSI_F;
    case Qt::Key_G: return kVK_ANSI_G;
    case Qt::Key_H: return kVK_ANSI_H;
    case Qt::Key_I: return kVK_ANSI_I;
    case Qt::Key_J: return kVK_ANSI_J;
    case Qt::Key_K: return kVK_ANSI_K;
    case Qt::Key_L: return kVK_ANSI_L;
    case Qt::Key_M: return kVK_ANSI_M;
    case Qt::Key_N: return kVK_ANSI_N;
    case Qt::Key_O: return kVK_ANSI_O;
    case Qt::Key_P: return kVK_ANSI_P;
    case Qt::Key_Q: return kVK_ANSI_Q;
    case Qt::Key_R: return kVK_ANSI_R;
    case Qt::Key_S: return kVK_ANSI_S;
    case Qt::Key_T: return kVK_ANSI_T;
    case Qt::Key_U: return kVK_ANSI_U;
    case Qt::Key_V: return kVK_ANSI_V;
    case Qt::Key_W: return kVK_ANSI_W;
    case Qt::Key_X: return kVK_ANSI_X;
    case Qt::Key_Y: return kVK_ANSI_Y;
    case Qt::Key_Z: return kVK_ANSI_Z;
    case Qt::Key_0: return kVK_ANSI_0;
    case Qt::Key_1: return kVK_ANSI_1;
    case Qt::Key_2: return kVK_ANSI_2;
    case Qt::Key_3: return kVK_ANSI_3;
    case Qt::Key_4: return kVK_ANSI_4;
    case Qt::Key_5: return kVK_ANSI_5;
    case Qt::Key_6: return kVK_ANSI_6;
    case Qt::Key_7: return kVK_ANSI_7;
    case Qt::Key_8: return kVK_ANSI_8;
    case Qt::Key_9: return kVK_ANSI_9;
    case Qt::Key_F1: return kVK_F1;
    case Qt::Key_F2: return kVK_F2;
    case Qt::Key_F3: return kVK_F3;
    case Qt::Key_F4: return kVK_F4;
    case Qt::Key_F5: return kVK_F5;
    case Qt::Key_F6: return kVK_F6;
    case Qt::Key_F7: return kVK_F7;
    case Qt::Key_F8: return kVK_F8;
    case Qt::Key_F9: return kVK_F9;
    case Qt::Key_F10: return kVK_F10;
    case Qt::Key_F11: return kVK_F11;
    case Qt::Key_F12: return kVK_F12;
    case Qt::Key_F13: return kVK_F13;
    case Qt::Key_F14: return kVK_F14;
    case Qt::Key_F15: return kVK_F15;
    case Qt::Key_F16: return kVK_F16;
    case Qt::Key_F17: return kVK_F17;
    case Qt::Key_F18: return kVK_F18;
    case Qt::Key_F19: return kVK_F19;
    case Qt::Key_F20: return kVK_F10;
    case Qt::Key_Return: return kVK_Return;
    case Qt::Key_Enter: return kVK_ANSI_KeypadEnter;
    case Qt::Key_Tab: return kVK_Tab;
    case Qt::Key_Space: return kVK_Space;
    case Qt::Key_Backspace: return kVK_Delete;
    case Qt::Key_Escape: return kVK_Escape;
    case Qt::Key_CapsLock: return kVK_CapsLock;
    case Qt::Key_Option: return kVK_Option;
    case Qt::Key_VolumeUp: return kVK_VolumeUp;
    case Qt::Key_VolumeDown: return kVK_VolumeDown;
    case Qt::Key_Help: return kVK_Help;
    case Qt::Key_Home: return kVK_Home;
    case Qt::Key_PageUp: return kVK_PageUp;
    case Qt::Key_Delete: return kVK_ForwardDelete;
    case Qt::Key_End: return kVK_End;
    case Qt::Key_PageDown: return kVK_PageDown;
    case Qt::Key_Left: return kVK_LeftArrow;
    case Qt::Key_Right: return kVK_RightArrow;
    case Qt::Key_Down: return kVK_DownArrow;
    case Qt::Key_Up: return kVK_UpArrow;
    default: return 0;
  }
}

quint32 GlobalAction::nativeModifiers(Qt::KeyboardModifiers modifiers)
{
  quint32 native = 0;
  if (modifiers & Qt::ShiftModifier)
    native |= shiftKey;
  if (modifiers & Qt::ControlModifier)
    native |= cmdKey;
  if (modifiers & Qt::AltModifier)
    native |= optionKey;
  if (modifiers & Qt::MetaModifier)
    native |= controlKey;
  return native;
}

#endif  // ifdef Q_OS_MAC

}  // namespace service

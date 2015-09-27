#include "GlobalActionHelper.h"

#include <QDebug>
#include <QApplication>

QHash<QPair<quint32, quint32>, QAction *> GlobalActionHelper::actions_;

void GlobalActionHelper::init () {
  qApp->installNativeEventFilter (new GlobalActionHelper);
}

bool GlobalActionHelper::makeGlobal (QAction *action) {
  QKeySequence hotKey = action->shortcut ();
  if (hotKey.isEmpty ()) {
    return true;
  }
  Qt::KeyboardModifiers allMods = Qt::ShiftModifier | Qt::ControlModifier |
                                  Qt::AltModifier | Qt::MetaModifier;
  Qt::Key key = hotKey.isEmpty () ?
                Qt::Key (0) :
                Qt::Key ((hotKey[0] ^ allMods) & hotKey[0]);
  Qt::KeyboardModifiers mods = hotKey.isEmpty () ?
                               Qt::KeyboardModifiers (0) :
                               Qt::KeyboardModifiers (hotKey[0] & allMods);
  const quint32 nativeKey = nativeKeycode (key);
  const quint32 nativeMods = nativeModifiers (mods);
  const bool res = registerHotKey (nativeKey, nativeMods);
  if (res) {
    actions_.insert (qMakePair (nativeKey, nativeMods), action);
  }
  else {
    qWarning () << "Failed to register global hotkey:" << hotKey.toString ();
  }
  return res;
}

bool GlobalActionHelper::removeGlobal (QAction *action) {
  QKeySequence hotKey = action->shortcut ();
  if (hotKey.isEmpty ()) {
    return true;
  }
  Qt::KeyboardModifiers allMods = Qt::ShiftModifier | Qt::ControlModifier |
                                  Qt::AltModifier | Qt::MetaModifier;
  Qt::Key key = hotKey.isEmpty () ?
                Qt::Key (0) :
                Qt::Key ((hotKey[0] ^ allMods) & hotKey[0]);
  Qt::KeyboardModifiers mods = hotKey.isEmpty () ?
                               Qt::KeyboardModifiers (0) :
                               Qt::KeyboardModifiers (hotKey[0] & allMods);
  const quint32 nativeKey = nativeKeycode (key);
  const quint32 nativeMods = nativeModifiers (mods);
  if (!actions_.contains (qMakePair (nativeKey, nativeMods))) {
    return true;
  }
  const bool res = unregisterHotKey (nativeKey, nativeMods);
  if (res) {
    actions_.remove (qMakePair (nativeKey, nativeMods));
  }
  else {
    qWarning () << "Failed to unregister global hotkey:" << hotKey.toString ();
  }
  return res;
}



#if defined(Q_OS_LINUX)
#  include <QX11Info>
#  include <X11/Xlib.h>
#  include <xcb/xcb_event.h>

static bool error = false;

static int customHandler (Display *display, XErrorEvent *event) {
  Q_UNUSED (display);
  switch (event->error_code) {
    case BadAccess:
    case BadValue:
    case BadWindow:
      if (event->request_code == 33 /* X_GrabKey */ ||
          event->request_code == 34 /* X_UngrabKey */) {
        error = true;
      }
    default:
      return 0;
  }
}

bool GlobalActionHelper::registerHotKey (quint32 nativeKey, quint32 nativeMods) {
  Display *display = QX11Info::display ();
  Window window = QX11Info::appRootWindow ();
  Bool owner = True;
  int pointer = GrabModeAsync;
  int keyboard = GrabModeAsync;
  error = false;
  int (*handler)(Display *display, XErrorEvent *event) = XSetErrorHandler (customHandler);
  XGrabKey (display, nativeKey, nativeMods, window, owner, pointer, keyboard);
  // allow numlock
  XGrabKey (display, nativeKey, nativeMods | Mod2Mask, window, owner, pointer, keyboard);
  XSync (display, False);
  XSetErrorHandler (handler);
  return !error;
}

bool GlobalActionHelper::unregisterHotKey (quint32 nativeKey, quint32 nativeMods) {
  Display *display = QX11Info::display ();
  Window window = QX11Info::appRootWindow ();
  error = false;
  int (*handler)(Display *display, XErrorEvent *event) = XSetErrorHandler (customHandler);
  XUngrabKey (display, nativeKey, nativeMods, window);
  // allow numlock
  XUngrabKey (display, nativeKey, nativeMods | Mod2Mask, window);
  XSync (display, False);
  XSetErrorHandler (handler);
  return !error;
}

bool GlobalActionHelper::nativeEventFilter (const QByteArray &eventType,
                                            void *message, long *result) {
  Q_UNUSED (eventType);
  Q_UNUSED (result);
  xcb_generic_event_t *event = static_cast<xcb_generic_event_t *>(message);
  if (event->response_type == XCB_KEY_PRESS) {
    xcb_key_press_event_t *keyEvent = static_cast<xcb_key_press_event_t *>(message);
    const quint32 keycode = keyEvent->detail;
    const quint32 modifiers = keyEvent->state & ~XCB_MOD_MASK_2;
    QAction *action = actions_.value (qMakePair (keycode, modifiers));
    if (action) {
      action->activate (QAction::Trigger);
    }
  }
  return false;
}

quint32 GlobalActionHelper::nativeKeycode (Qt::Key key) {
  Display *display = QX11Info::display ();
  KeySym keySym =  XStringToKeysym (qPrintable (QKeySequence (key).toString ()));
  return XKeysymToKeycode (display, keySym);
}

quint32 GlobalActionHelper::nativeModifiers (Qt::KeyboardModifiers modifiers) {
  quint32 native = 0;
  if (modifiers & Qt::ShiftModifier) {
    native |= ShiftMask;
  }
  if (modifiers & Qt::ControlModifier) {
    native |= ControlMask;
  }
  if (modifiers & Qt::AltModifier) {
    native |= Mod1Mask;
  }
  if (modifiers & Qt::MetaModifier) {
    native |= Mod4Mask;
  }
  return native;
}



#elif defined(Q_OS_WIN)
#  include <qt_windows.h>


bool GlobalActionHelper::registerHotKey (quint32 nativeKey, quint32 nativeMods) {
  return RegisterHotKey (0, nativeMods ^ nativeKey, nativeMods, nativeKey);
}

bool GlobalActionHelper::unregisterHotKey (quint32 nativeKey, quint32 nativeMods) {
  return UnregisterHotKey (0, nativeMods ^ nativeKey);
}

bool GlobalActionHelper::nativeEventFilter (const QByteArray &eventType,
                                            void *message, long *result) {
  Q_UNUSED (eventType);
  Q_UNUSED (result);
  MSG *msg = static_cast<MSG *>(message);
  if (msg->message == WM_HOTKEY) {
    const quint32 keycode = HIWORD (msg->lParam);
    const quint32 modifiers = LOWORD (msg->lParam);
    QAction *action = actions_.value (qMakePair (keycode, modifiers));
    if (action) {
      action->activate (QAction::Trigger);
    }
  }
  return false;
}

quint32 GlobalActionHelper::nativeKeycode (Qt::Key key) {
  switch (key) {
    case Qt::Key_Escape:
      return VK_ESCAPE;
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
      return VK_TAB;
    case Qt::Key_Backspace:
      return VK_BACK;
    case Qt::Key_Return:
    case Qt::Key_Enter:
      return VK_RETURN;
    case Qt::Key_Insert:
      return VK_INSERT;
    case Qt::Key_Delete:
      return VK_DELETE;
    case Qt::Key_Pause:
      return VK_PAUSE;
    case Qt::Key_Print:
      return VK_PRINT;
    case Qt::Key_Clear:
      return VK_CLEAR;
    case Qt::Key_Home:
      return VK_HOME;
    case Qt::Key_End:
      return VK_END;
    case Qt::Key_Left:
      return VK_LEFT;
    case Qt::Key_Up:
      return VK_UP;
    case Qt::Key_Right:
      return VK_RIGHT;
    case Qt::Key_Down:
      return VK_DOWN;
    case Qt::Key_PageUp:
      return VK_PRIOR;
    case Qt::Key_PageDown:
      return VK_NEXT;
    case Qt::Key_F1:
      return VK_F1;
    case Qt::Key_F2:
      return VK_F2;
    case Qt::Key_F3:
      return VK_F3;
    case Qt::Key_F4:
      return VK_F4;
    case Qt::Key_F5:
      return VK_F5;
    case Qt::Key_F6:
      return VK_F6;
    case Qt::Key_F7:
      return VK_F7;
    case Qt::Key_F8:
      return VK_F8;
    case Qt::Key_F9:
      return VK_F9;
    case Qt::Key_F10:
      return VK_F10;
    case Qt::Key_F11:
      return VK_F11;
    case Qt::Key_F12:
      return VK_F12;
    case Qt::Key_F13:
      return VK_F13;
    case Qt::Key_F14:
      return VK_F14;
    case Qt::Key_F15:
      return VK_F15;
    case Qt::Key_F16:
      return VK_F16;
    case Qt::Key_F17:
      return VK_F17;
    case Qt::Key_F18:
      return VK_F18;
    case Qt::Key_F19:
      return VK_F19;
    case Qt::Key_F20:
      return VK_F20;
    case Qt::Key_F21:
      return VK_F21;
    case Qt::Key_F22:
      return VK_F22;
    case Qt::Key_F23:
      return VK_F23;
    case Qt::Key_F24:
      return VK_F24;
    case Qt::Key_Space:
      return VK_SPACE;
    case Qt::Key_Asterisk:
      return VK_MULTIPLY;
    case Qt::Key_Plus:
      return VK_ADD;
    case Qt::Key_Comma:
      return VK_SEPARATOR;
    case Qt::Key_Minus:
      return VK_SUBTRACT;
    case Qt::Key_Slash:
      return VK_DIVIDE;
    case Qt::Key_MediaNext:
      return VK_MEDIA_NEXT_TRACK;
    case Qt::Key_MediaPrevious:
      return VK_MEDIA_PREV_TRACK;
    case Qt::Key_MediaPlay:
      return VK_MEDIA_PLAY_PAUSE;
    case Qt::Key_MediaStop:
      return VK_MEDIA_STOP;
    case Qt::Key_VolumeDown:
      return VK_VOLUME_DOWN;
    case Qt::Key_VolumeUp:
      return VK_VOLUME_UP;
    case Qt::Key_VolumeMute:
      return VK_VOLUME_MUTE;

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
    case Qt::Key_9:
      return key;

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
    case Qt::Key_Z:
      return key;

    default:
      return 0;
  }
}

quint32 GlobalActionHelper::nativeModifiers (Qt::KeyboardModifiers modifiers) {
  // MOD_ALT, MOD_CONTROL, (MOD_KEYUP), MOD_SHIFT, MOD_WIN
  quint32 native = 0;
  if (modifiers & Qt::ShiftModifier) {
    native |= MOD_SHIFT;
  }
  if (modifiers & Qt::ControlModifier) {
    native |= MOD_CONTROL;
  }
  if (modifiers & Qt::AltModifier) {
    native |= MOD_ALT;
  }
  if (modifiers & Qt::MetaModifier) {
    native |= MOD_WIN;
  }
  //if (modifiers & Qt::KeypadModifier)
  //if (modifiers & Qt::GroupSwitchModifier)
  return native;
}
#endif

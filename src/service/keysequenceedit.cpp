#include "keysequenceedit.h"

#include <QKeyEvent>

namespace service
{
KeySequenceEdit::KeySequenceEdit(QWidget *parent)
  : QLineEdit(parent)
{
  setPlaceholderText(tr("Press shortcut"));
  setFocusPolicy(Qt::StrongFocus);
  setAttribute(Qt::WA_MacShowFocusRect, true);
  setAttribute(Qt::WA_InputMethodEnabled, false);
}

const QKeySequence &KeySequenceEdit::keySequence() const
{
  return current_;
}

void KeySequenceEdit::setKeySequence(const QKeySequence &newKeySequence)
{
  setKeySequence(newKeySequence, true);
}

void KeySequenceEdit::setKeySequence(const QKeySequence &current,
                                     bool updateFallback)
{
  current_ = current;
  if (updateFallback)
    fallback_ = current;
  setText(current_.toString(QKeySequence::NativeText));
}

bool KeySequenceEdit::event(QEvent *e)
{
  switch (e->type()) {
    case QEvent::Shortcut: return true;
    case QEvent::ShortcutOverride: e->accept(); return true;
    default: break;
  }

  return QWidget::event(e);
}

void KeySequenceEdit::keyPressEvent(QKeyEvent *event)
{
  const auto key = event->key();
  if (key == Qt::Key_Control || key == Qt::Key_Meta || key == Qt::Key_Alt ||
      key == Qt::Key_Shift || key == Qt::Key_unknown) {
    return;
  }

  if (event->key() == Qt::Key_Escape) {
    setKeySequence(fallback_, false);
    event->accept();
    return;
  }

  if (event->key() == Qt::Key_Backspace) {
    setKeySequence({}, false);
    event->accept();
    return;
  }

  QKeySequence seq = event->modifiers() + event->key();
  setKeySequence(seq, false);
  event->accept();
}

}  // namespace service

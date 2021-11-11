#pragma once

#include <QLineEdit>

namespace service
{
class KeySequenceEdit : public QLineEdit
{
public:
  KeySequenceEdit(QWidget *parent = nullptr);

  const QKeySequence &keySequence() const;
  void setKeySequence(const QKeySequence &newKeySequence);
  bool event(QEvent *event) override;

protected:
  void keyPressEvent(QKeyEvent *event) override;

private:
  void setKeySequence(const QKeySequence &current, bool updateFallback);
  QKeySequence current_;
  QKeySequence fallback_;
};

}  // namespace service

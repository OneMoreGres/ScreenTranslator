#pragma once

#include "stfwd.h"

#include <QWidget>

class QScreen;

class CaptureAreaSelector : public QWidget
{
  Q_OBJECT

public:
  explicit CaptureAreaSelector(Capturer &capturer);

  void setScreen(QScreen &screen);

protected:
  void showEvent(QShowEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void paintEvent(QPaintEvent *event) override;

private:
  Capturer &capturer_;
  QPixmap pixmap_;
  QPoint startSelectPos_;
  QPoint currentSelectPos_;
};

#pragma once

#include "stfwd.h"

#include <QWidget>

class QScreen;

class CaptureAreaSelector : public QWidget
{
  Q_OBJECT

public:
  CaptureAreaSelector(Capturer &capturer, const Settings &settings);

  void setScreen(QScreen &screen);

protected:
  void showEvent(QShowEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void paintEvent(QPaintEvent *event) override;

private:
  void updateHelp();

  Capturer &capturer_;
  const Settings &settings_;
  QPixmap pixmap_;
  QPoint startSelectPos_;
  QPoint currentSelectPos_;
  QString help_;
  QRect currentHelpRect_;
  std::vector<QRect> helpRects_;
};

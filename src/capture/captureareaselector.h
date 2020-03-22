#pragma once

#include "stfwd.h"

#include <QWidget>

class CaptureAreaSelector : public QWidget
{
  Q_OBJECT

public:
  CaptureAreaSelector(Capturer &capturer, const Settings &settings);

  void activate();
  void setPixmap(const QPixmap &pixmap);
  void setScreenRects(const std::vector<QRect> &screens);
  void updateSettings();

protected:
  void showEvent(QShowEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void paintEvent(QPaintEvent *event) override;

private:
  struct HelpRect {
    QRect current;
    std::vector<QRect> possible;
  };

  bool updateCurrentHelpRects();

  Capturer &capturer_;
  const Settings &settings_;
  QPixmap pixmap_;
  QPoint startSelectPos_;
  QPoint currentSelectPos_;
  QString help_;
  std::vector<HelpRect> helpRects_;
};

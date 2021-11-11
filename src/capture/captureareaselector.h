#pragma once

#include "stfwd.h"

#include <QWidget>

class QMenu;

class CaptureAreaSelector : public QWidget
{
  Q_OBJECT

public:
  CaptureAreaSelector(Capturer &capturer, const Settings &settings,
                      const CommonModels &models, const QPixmap &pixmap,
                      const QPoint &pixmapOffset);
  ~CaptureAreaSelector();

  void activate();
  bool hasLocked() const;
  void captureLocked();
  void setScreenRects(const std::vector<QRect> &screens);
  void updateSettings();

protected:
  void showEvent(QShowEvent *event) override;
  void hideEvent(QHideEvent *event) override;
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
  void capture(CaptureArea &area, uint generation);
  void captureAll();
  void cancel();
  void updateCursorShape(const QPoint &pos);

  bool updateCurrentHelpRects();
  void drawHelpRects(QPainter &painter, const HelpRect &rect) const;

  void customize(const std::shared_ptr<CaptureArea> &area);
  void applyEditor();
  void drawCaptureArea(QPainter &painter, const CaptureArea &area) const;

  Capturer &capturer_;
  const Settings &settings_;
  const QPixmap &pixmap_;
  const QPoint &pixmapOffset_;
  Generation generation_{};
  QPoint startSelectPos_;
  QPoint currentSelectPos_;
  QString help_;
  std::vector<HelpRect> helpRects_;
  std::vector<std::shared_ptr<CaptureArea>> areas_;
  std::weak_ptr<CaptureArea> edited_;
  std::unique_ptr<CaptureAreaEditor> editor_;
  QMenu *contextMenu_;
};

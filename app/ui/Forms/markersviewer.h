#ifndef MARKERSVIEWER_H
#define MARKERSVIEWER_H

#include <QDockWidget>

#include "project/media.h"

namespace Ui {
class MarkersViewer;
}

class MarkersViewer : public QDockWidget
{
  Q_OBJECT

public:
  explicit MarkersViewer(QWidget *parent = nullptr);
  ~MarkersViewer();

  bool setMedia(const MediaPtr& mda);
  void refresh();
  void reset();


private:
  Ui::MarkersViewer *ui{};
  MediaWPtr media_{};
};

#endif // MARKERSVIEWER_H

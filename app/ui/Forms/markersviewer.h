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
  ~MarkersViewer() ;

  /**
   * Set this viewers media item
   * @param mda
   * @return true==media set,  false==null ptr or invalid type
   */
  bool setMedia(const MediaPtr& mda);
  /**
   * Rebuild the viewer
   * @param filter a simple 'contains' search function in marker name or comment
   */
  void refresh(const QString& filter="");
  /**
   * Clear this viewers media item
   */
  void reset();


private:
  Ui::MarkersViewer *ui{};
  MediaWPtr media_{};
};

#endif // MARKERSVIEWER_H

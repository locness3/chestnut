#ifndef MARKERSVIEWER_H
#define MARKERSVIEWER_H

#include <QDockWidget>

namespace Ui {
  class MarkersViewer;
}

class MarkersViewer : public QDockWidget
{
    Q_OBJECT

  public:
    explicit MarkersViewer(QWidget *parent = nullptr);
    ~MarkersViewer();

  private:
    Ui::MarkersViewer *ui;
};

#endif // MARKERSVIEWER_H

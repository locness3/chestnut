#ifndef MARKERWIDGET_H
#define MARKERWIDGET_H

#include <QWidget>

#include "project/marker.h"

namespace Ui {
  class MarkerWidget;
}

class MarkerWidget : public QWidget
{
    Q_OBJECT

  public:
    MarkerWidget(Marker mark, QWidget *parent = nullptr);
    ~MarkerWidget();

  private:
    Ui::MarkerWidget *ui;
    Marker marker_;
};

#endif // MARKERWIDGET_H

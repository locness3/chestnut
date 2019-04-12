#ifndef MARKERWIDGET_H
#define MARKERWIDGET_H

#include <QWidget>

namespace Ui {
  class MarkerWidget;
}

class MarkerWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit MarkerWidget(QWidget *parent = nullptr);
    ~MarkerWidget();

  private:
    Ui::MarkerWidget *ui;
};

#endif // MARKERWIDGET_H

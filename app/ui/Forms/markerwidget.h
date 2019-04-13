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
    MarkerWidget(MarkerPtr mark, QWidget *parent = nullptr);
    ~MarkerWidget();

  private slots:
    void onValChange(const QString& val);
    void onCommentChanged();

  private:
    Ui::MarkerWidget *ui;
    MarkerPtr marker_;
};

#endif // MARKERWIDGET_H

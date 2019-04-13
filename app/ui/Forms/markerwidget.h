#ifndef MARKERWIDGET_H
#define MARKERWIDGET_H

#include <QWidget>

#include "project/marker.h"

namespace Ui {
  class MarkerWidget;
}

class MarkerIcon : public QWidget
{
    Q_OBJECT
  public:
    explicit MarkerIcon(QWidget *parent = nullptr);
    QColor clr_{};
  protected:
    /*
     * Draw the icon of the grabbed frame with colored hint
     */
    void paintEvent(QPaintEvent *event) override;
    /*
     * Open up a dialog
     */
    void mouseDoubleClickEvent(QMouseEvent *event) override;
};

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
    MarkerIcon* marker_icon_;
};

#endif // MARKERWIDGET_H

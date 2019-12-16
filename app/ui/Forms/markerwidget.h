/*
 * Chestnut. Chestnut is a free non-linear video editor for Linux.
 * Copyright (C) 2019
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MARKERWIDGET_H
#define MARKERWIDGET_H

#include <QWidget>

#include "project/marker.h"
#include "ui/labelslider.h"

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
    /**
     * @brief Draw the icon of the grabbed frame with colored hint
     */
    void paintEvent(QPaintEvent *event) override;
    /**
     * @brief Open up a dialog
     */
    void mouseDoubleClickEvent(QMouseEvent *event) override;
};

class MarkerWidget : public QWidget
{
    Q_OBJECT

  public:
    MarkerWidget(MarkerWPtr mark, QWidget *parent = nullptr);
    ~MarkerWidget() override;

  signals:
    void changed();
  private slots:
    void onValChange(const QString& val);
    void onCommentChanged();
    void onSliderChanged();

  private:
    Ui::MarkerWidget *ui {nullptr};
    MarkerWPtr marker_;
    MarkerIcon* marker_icon_ {nullptr};
    LabelSlider* in_slider_ {nullptr};
    LabelSlider* out_slider_ {nullptr};
};

#endif // MARKERWIDGET_H

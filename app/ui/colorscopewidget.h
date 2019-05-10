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

#ifndef COLORSCOPEWIDGET_H
#define COLORSCOPEWIDGET_H

#include <QWidget>

namespace ui {

  class ColorScopeWidget : public QWidget
  {
      Q_OBJECT
    public:
      explicit ColorScopeWidget(QWidget *parent = nullptr);
      /**
       * @brief Update the image used to draw scope
       * @param img Image to be drawn from
       */
      void updateImage(QImage img);
      int mode_{0};
    protected:
      void paintEvent(QPaintEvent *event) override;
    private:
      QImage img_;
  };
}

#endif // COLORSCOPEWIDGET_H

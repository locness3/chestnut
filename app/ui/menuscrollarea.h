/*
 * Chestnut. Chestnut is a free non-linear video editor for Linux.
 * Copyright (C) 2020 Jonathan Noble
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

#ifndef MENUSCROLLAREA_H
#define MENUSCROLLAREA_H

#include <QScrollArea>
#include <QMouseEvent>

namespace chestnut::ui
{
  /**
   * @brief ScrollArea with a menu available on right-mouseclick
   */
  class MenuScrollArea : public QScrollArea
  {
      Q_OBJECT
    public:
      explicit MenuScrollArea(QWidget* parent = nullptr);

    protected:
      void mousePressEvent(QMouseEvent* event) override;

    signals:
      void setZoom(const double value);
      void clear();
    private:
      void showContextMenu(const QPoint& point);
    private slots:
      void setMenuZoom(QAction* action);
      void clearArea();
  };
}

#endif // MENUSCROLLAREA_H

/* 
 * Olive. Olive is a free non-linear video editor for Windows, macOS, and Linux.
 * Copyright (C) 2018  {{ organization }}
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
 *along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef VIEWERCONTAINER_H
#define VIEWERCONTAINER_H

#include <QScrollArea>

#include "chestnut.h"

class Viewer;
class ViewerWidget;

class ViewerContainer : public QScrollArea
{
    Q_OBJECT
  public:
    bool fit {true};
    double zoom {1.0};
    Viewer* viewer {nullptr};
    ViewerWidget* child {nullptr};

    explicit ViewerContainer(QWidget *parent = nullptr);

    void dragScrollPress(const QPoint& p);
    void dragScrollMove(const QPoint& p);
    void adjust();


  protected:
    void resizeEvent(QResizeEvent *event) override;

  private:
    QWidget* area {nullptr};
    chestnut::Coordinate_T drag_start_;
    int horiz_start {0};
    int vert_start {0};
};

#endif // VIEWERCONTAINER_H
